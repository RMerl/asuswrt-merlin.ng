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

#ifndef MISC_H__
#define MISC_H__

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
#define MISC_RESCAL_CTRL                         0x00000000 /* Configuration for RESCAL IP */
#define MISC_RESCAL_VALUE                        0x00000004 /* Status for RESCAL IP */
#define MISC_TEMPS_CTRL                          0x00000010 /* Configuration for TEMPS IP */
#define MISC_TEMPS_DATA                          0x00000018 /* TEMPS IP output data */
#define MISC_PMD_INT_TYPE_CTL                    0x00000020 /* Interrupt-type control */
#define MISC_PMD_INT_STATUS                      0x00000024 /* Top-Level interrupt status */
#define MISC_PMD_INT_EN                          0x00000028 /* Top-level interrupt enable */
#define MISC_CPU_RAM_PSM_VDD                     0x0000002c /* PSM VDD control for CPU internal memory */
#define MISC_PMD_RAM_PSM_VDD                     0x00000030 /* PSM VDD control for PMD main memory */
#define MISC_MISC_STAT_BURST_COUNT               0x00000034 /* The number of BE rising edges since statistic was reset */
#define MISC_MISC_STAT_BURST_ON_TIME             0x00000038 /* The total number of system clocks BE was high */
#define MISC_MISC_STAT_BURST_ON_MAX              0x0000003c /* The maximum number of system clocks a single BE was high */
#define MISC_MISC_STAT_BURST_ON_MIN              0x00000040 /* The minimum number of system clocks a single BE was high */
#define MISC_MISC_STAT_BURST_OFF_TIME            0x00000044 /* The total number of system clocks BE was low */
#define MISC_MISC_STAT_BURST_OFF_MAX             0x00000048 /* The maximum number of system clocks a single BE was low */
#define MISC_MISC_STAT_BURST_OFF_MIN             0x0000004c /* The minimum number of system clocks a single BE was low */

/***************************************************************************
 *RESCAL_CTRL - Configuration for RESCAL IP
 ***************************************************************************/
/* MISC :: RESCAL_CTRL :: RESCAL_DONE [31:31] */
#define MISC_RESCAL_CTRL_RESCAL_DONE_MASK                          0x80000000
#define MISC_RESCAL_CTRL_RESCAL_DONE_ALIGN                         0
#define MISC_RESCAL_CTRL_RESCAL_DONE_BITS                          1
#define MISC_RESCAL_CTRL_RESCAL_DONE_SHIFT                         31
#define MISC_RESCAL_CTRL_RESCAL_DONE_DEFAULT                       0

/* MISC :: RESCAL_CTRL :: reserved0 [30:05] */
#define MISC_RESCAL_CTRL_reserved0_MASK                            0x7fffffe0
#define MISC_RESCAL_CTRL_reserved0_ALIGN                           0
#define MISC_RESCAL_CTRL_reserved0_BITS                            26
#define MISC_RESCAL_CTRL_reserved0_SHIFT                           5

/* MISC :: RESCAL_CTRL :: RESCAL_ENABLE [04:04] */
#define MISC_RESCAL_CTRL_RESCAL_ENABLE_MASK                        0x00000010
#define MISC_RESCAL_CTRL_RESCAL_ENABLE_ALIGN                       0
#define MISC_RESCAL_CTRL_RESCAL_ENABLE_BITS                        1
#define MISC_RESCAL_CTRL_RESCAL_ENABLE_SHIFT                       4
#define MISC_RESCAL_CTRL_RESCAL_ENABLE_DEFAULT                     0

/* MISC :: RESCAL_CTRL :: reserved1 [03:02] */
#define MISC_RESCAL_CTRL_reserved1_MASK                            0x0000000c
#define MISC_RESCAL_CTRL_reserved1_ALIGN                           0
#define MISC_RESCAL_CTRL_reserved1_BITS                            2
#define MISC_RESCAL_CTRL_reserved1_SHIFT                           2

/* MISC :: RESCAL_CTRL :: RESCAL_RESETB [01:01] */
#define MISC_RESCAL_CTRL_RESCAL_RESETB_MASK                        0x00000002
#define MISC_RESCAL_CTRL_RESCAL_RESETB_ALIGN                       0
#define MISC_RESCAL_CTRL_RESCAL_RESETB_BITS                        1
#define MISC_RESCAL_CTRL_RESCAL_RESETB_SHIFT                       1
#define MISC_RESCAL_CTRL_RESCAL_RESETB_DEFAULT                     0

/* MISC :: RESCAL_CTRL :: RESCAL_PWRDN [00:00] */
#define MISC_RESCAL_CTRL_RESCAL_PWRDN_MASK                         0x00000001
#define MISC_RESCAL_CTRL_RESCAL_PWRDN_ALIGN                        0
#define MISC_RESCAL_CTRL_RESCAL_PWRDN_BITS                         1
#define MISC_RESCAL_CTRL_RESCAL_PWRDN_SHIFT                        0
#define MISC_RESCAL_CTRL_RESCAL_PWRDN_DEFAULT                      1

/***************************************************************************
 *RESCAL_VALUE - Status for RESCAL IP
 ***************************************************************************/
/* MISC :: RESCAL_VALUE :: reserved0 [31:03] */
#define MISC_RESCAL_VALUE_reserved0_MASK                           0xfffffff8
#define MISC_RESCAL_VALUE_reserved0_ALIGN                          0
#define MISC_RESCAL_VALUE_reserved0_BITS                           29
#define MISC_RESCAL_VALUE_reserved0_SHIFT                          3

/* MISC :: RESCAL_VALUE :: RESCAL_VALUE [02:00] */
#define MISC_RESCAL_VALUE_RESCAL_VALUE_MASK                        0x00000007
#define MISC_RESCAL_VALUE_RESCAL_VALUE_ALIGN                       0
#define MISC_RESCAL_VALUE_RESCAL_VALUE_BITS                        3
#define MISC_RESCAL_VALUE_RESCAL_VALUE_SHIFT                       0
#define MISC_RESCAL_VALUE_RESCAL_VALUE_DEFAULT                     0

/***************************************************************************
 *TEMPS_CTRL - Configuration for TEMPS IP
 ***************************************************************************/
/* MISC :: TEMPS_CTRL :: reserved0 [31:05] */
#define MISC_TEMPS_CTRL_reserved0_MASK                             0xffffffe0
#define MISC_TEMPS_CTRL_reserved0_ALIGN                            0
#define MISC_TEMPS_CTRL_reserved0_BITS                             27
#define MISC_TEMPS_CTRL_reserved0_SHIFT                            5

/* MISC :: TEMPS_CTRL :: TEMPS_DONE [04:04] */
#define MISC_TEMPS_CTRL_TEMPS_DONE_MASK                            0x00000010
#define MISC_TEMPS_CTRL_TEMPS_DONE_ALIGN                           0
#define MISC_TEMPS_CTRL_TEMPS_DONE_BITS                            1
#define MISC_TEMPS_CTRL_TEMPS_DONE_SHIFT                           4
#define MISC_TEMPS_CTRL_TEMPS_DONE_DEFAULT                         0

/* MISC :: TEMPS_CTRL :: reserved1 [03:00] */
#define MISC_TEMPS_CTRL_reserved1_MASK                             0x0000000f
#define MISC_TEMPS_CTRL_reserved1_ALIGN                            0
#define MISC_TEMPS_CTRL_reserved1_BITS                             4
#define MISC_TEMPS_CTRL_reserved1_SHIFT                            0

/***************************************************************************
 *TEMPS_DATA - TEMPS IP output data
 ***************************************************************************/
/* MISC :: TEMPS_DATA :: reserved0 [31:10] */
#define MISC_TEMPS_DATA_reserved0_MASK                             0xfffffc00
#define MISC_TEMPS_DATA_reserved0_ALIGN                            0
#define MISC_TEMPS_DATA_reserved0_BITS                             22
#define MISC_TEMPS_DATA_reserved0_SHIFT                            10

/* MISC :: TEMPS_DATA :: TEMPS_DATA [09:00] */
#define MISC_TEMPS_DATA_TEMPS_DATA_MASK                            0x000003ff
#define MISC_TEMPS_DATA_TEMPS_DATA_ALIGN                           0
#define MISC_TEMPS_DATA_TEMPS_DATA_BITS                            10
#define MISC_TEMPS_DATA_TEMPS_DATA_SHIFT                           0
#define MISC_TEMPS_DATA_TEMPS_DATA_DEFAULT                         0

/***************************************************************************
 *PMD_INT_TYPE_CTL - Interrupt-type control
 ***************************************************************************/
/* MISC :: PMD_INT_TYPE_CTL :: reserved0 [31:09] */
#define MISC_PMD_INT_TYPE_CTL_reserved0_MASK                       0xfffffe00
#define MISC_PMD_INT_TYPE_CTL_reserved0_ALIGN                      0
#define MISC_PMD_INT_TYPE_CTL_reserved0_BITS                       23
#define MISC_PMD_INT_TYPE_CTL_reserved0_SHIFT                      9

/* MISC :: PMD_INT_TYPE_CTL :: pmd_ldc_stat_done_int_type [08:08] */
#define MISC_PMD_INT_TYPE_CTL_pmd_ldc_stat_done_int_type_MASK      0x00000100
#define MISC_PMD_INT_TYPE_CTL_pmd_ldc_stat_done_int_type_ALIGN     0
#define MISC_PMD_INT_TYPE_CTL_pmd_ldc_stat_done_int_type_BITS      1
#define MISC_PMD_INT_TYPE_CTL_pmd_ldc_stat_done_int_type_SHIFT     8
#define MISC_PMD_INT_TYPE_CTL_pmd_ldc_stat_done_int_type_DEFAULT   0

/* MISC :: PMD_INT_TYPE_CTL :: pmd_rxsd_debug_out_int_type [07:06] */
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_debug_out_int_type_MASK     0x000000c0
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_debug_out_int_type_ALIGN    0
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_debug_out_int_type_BITS     2
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_debug_out_int_type_SHIFT    6
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_debug_out_int_type_DEFAULT  3

/* MISC :: PMD_INT_TYPE_CTL :: pmd_pm_debug_out_int_type [05:04] */
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_debug_out_int_type_MASK       0x00000030
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_debug_out_int_type_ALIGN      0
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_debug_out_int_type_BITS       2
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_debug_out_int_type_SHIFT      4
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_debug_out_int_type_DEFAULT    3

/* MISC :: PMD_INT_TYPE_CTL :: pmd_rxsd_value_int_type [03:02] */
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_value_int_type_MASK         0x0000000c
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_value_int_type_ALIGN        0
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_value_int_type_BITS         2
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_value_int_type_SHIFT        2
#define MISC_PMD_INT_TYPE_CTL_pmd_rxsd_value_int_type_DEFAULT      3

/* MISC :: PMD_INT_TYPE_CTL :: pmd_pm_value_int_type [01:00] */
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_value_int_type_MASK           0x00000003
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_value_int_type_ALIGN          0
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_value_int_type_BITS           2
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_value_int_type_SHIFT          0
#define MISC_PMD_INT_TYPE_CTL_pmd_pm_value_int_type_DEFAULT        3

/***************************************************************************
 *PMD_INT_STATUS - Top-Level interrupt status
 ***************************************************************************/
/* MISC :: PMD_INT_STATUS :: reserved0 [31:30] */
#define MISC_PMD_INT_STATUS_reserved0_MASK                         0xc0000000
#define MISC_PMD_INT_STATUS_reserved0_ALIGN                        0
#define MISC_PMD_INT_STATUS_reserved0_BITS                         2
#define MISC_PMD_INT_STATUS_reserved0_SHIFT                        30

/* MISC :: PMD_INT_STATUS :: pmd_tmon_int [29:29] */
#define MISC_PMD_INT_STATUS_pmd_tmon_int_MASK                      0x20000000
#define MISC_PMD_INT_STATUS_pmd_tmon_int_ALIGN                     0
#define MISC_PMD_INT_STATUS_pmd_tmon_int_BITS                      1
#define MISC_PMD_INT_STATUS_pmd_tmon_int_SHIFT                     29
#define MISC_PMD_INT_STATUS_pmd_tmon_int_DEFAULT                   0

/* MISC :: PMD_INT_STATUS :: pmd_rxsd_debug_out_int [28:28] */
#define MISC_PMD_INT_STATUS_pmd_rxsd_debug_out_int_MASK            0x10000000
#define MISC_PMD_INT_STATUS_pmd_rxsd_debug_out_int_ALIGN           0
#define MISC_PMD_INT_STATUS_pmd_rxsd_debug_out_int_BITS            1
#define MISC_PMD_INT_STATUS_pmd_rxsd_debug_out_int_SHIFT           28
#define MISC_PMD_INT_STATUS_pmd_rxsd_debug_out_int_DEFAULT         0

/* MISC :: PMD_INT_STATUS :: pmd_pm_debug_out_int [27:27] */
#define MISC_PMD_INT_STATUS_pmd_pm_debug_out_int_MASK              0x08000000
#define MISC_PMD_INT_STATUS_pmd_pm_debug_out_int_ALIGN             0
#define MISC_PMD_INT_STATUS_pmd_pm_debug_out_int_BITS              1
#define MISC_PMD_INT_STATUS_pmd_pm_debug_out_int_SHIFT             27
#define MISC_PMD_INT_STATUS_pmd_pm_debug_out_int_DEFAULT           0

/* MISC :: PMD_INT_STATUS :: pmd_ldc_acl_mod_unlocked_int [26:26] */
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_unlocked_int_MASK      0x04000000
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_unlocked_int_ALIGN     0
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_unlocked_int_BITS      1
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_unlocked_int_SHIFT     26
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_unlocked_int_DEFAULT   0

/* MISC :: PMD_INT_STATUS :: pmd_ldc_acl_mod_locked_int [25:25] */
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_locked_int_MASK        0x02000000
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_locked_int_ALIGN       0
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_locked_int_BITS        1
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_locked_int_SHIFT       25
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_mod_locked_int_DEFAULT     0

/* MISC :: PMD_INT_STATUS :: pmd_ldc_acl_bias_unlocked_int [24:24] */
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_unlocked_int_MASK     0x01000000
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_unlocked_int_ALIGN    0
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_unlocked_int_BITS     1
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_unlocked_int_SHIFT    24
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_unlocked_int_DEFAULT  0

/* MISC :: PMD_INT_STATUS :: pmd_ldc_acl_bias_locked_int [23:23] */
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_locked_int_MASK       0x00800000
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_locked_int_ALIGN      0
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_locked_int_BITS       1
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_locked_int_SHIFT      23
#define MISC_PMD_INT_STATUS_pmd_ldc_acl_bias_locked_int_DEFAULT    0

/* MISC :: PMD_INT_STATUS :: pmd_rxsd_value_int [22:22] */
#define MISC_PMD_INT_STATUS_pmd_rxsd_value_int_MASK                0x00400000
#define MISC_PMD_INT_STATUS_pmd_rxsd_value_int_ALIGN               0
#define MISC_PMD_INT_STATUS_pmd_rxsd_value_int_BITS                1
#define MISC_PMD_INT_STATUS_pmd_rxsd_value_int_SHIFT               22
#define MISC_PMD_INT_STATUS_pmd_rxsd_value_int_DEFAULT             0

/* MISC :: PMD_INT_STATUS :: pmd_pm_value_int [21:21] */
#define MISC_PMD_INT_STATUS_pmd_pm_value_int_MASK                  0x00200000
#define MISC_PMD_INT_STATUS_pmd_pm_value_int_ALIGN                 0
#define MISC_PMD_INT_STATUS_pmd_pm_value_int_BITS                  1
#define MISC_PMD_INT_STATUS_pmd_pm_value_int_SHIFT                 21
#define MISC_PMD_INT_STATUS_pmd_pm_value_int_DEFAULT               0

/* MISC :: PMD_INT_STATUS :: pmd_chip_sd_int [20:20] */
#define MISC_PMD_INT_STATUS_pmd_chip_sd_int_MASK                   0x00100000
#define MISC_PMD_INT_STATUS_pmd_chip_sd_int_ALIGN                  0
#define MISC_PMD_INT_STATUS_pmd_chip_sd_int_BITS                   1
#define MISC_PMD_INT_STATUS_pmd_chip_sd_int_SHIFT                  20
#define MISC_PMD_INT_STATUS_pmd_chip_sd_int_DEFAULT                0

/* MISC :: PMD_INT_STATUS :: pmd_ldc_stat_done_int [19:19] */
#define MISC_PMD_INT_STATUS_pmd_ldc_stat_done_int_MASK             0x00080000
#define MISC_PMD_INT_STATUS_pmd_ldc_stat_done_int_ALIGN            0
#define MISC_PMD_INT_STATUS_pmd_ldc_stat_done_int_BITS             1
#define MISC_PMD_INT_STATUS_pmd_ldc_stat_done_int_SHIFT            19
#define MISC_PMD_INT_STATUS_pmd_ldc_stat_done_int_DEFAULT          0

/* MISC :: PMD_INT_STATUS :: pmd_chip_mb_written_int [18:18] */
#define MISC_PMD_INT_STATUS_pmd_chip_mb_written_int_MASK           0x00040000
#define MISC_PMD_INT_STATUS_pmd_chip_mb_written_int_ALIGN          0
#define MISC_PMD_INT_STATUS_pmd_chip_mb_written_int_BITS           1
#define MISC_PMD_INT_STATUS_pmd_chip_mb_written_int_SHIFT          18
#define MISC_PMD_INT_STATUS_pmd_chip_mb_written_int_DEFAULT        0

/* MISC :: PMD_INT_STATUS :: pmd_chip_los_detect_int [17:17] */
#define MISC_PMD_INT_STATUS_pmd_chip_los_detect_int_MASK           0x00020000
#define MISC_PMD_INT_STATUS_pmd_chip_los_detect_int_ALIGN          0
#define MISC_PMD_INT_STATUS_pmd_chip_los_detect_int_BITS           1
#define MISC_PMD_INT_STATUS_pmd_chip_los_detect_int_SHIFT          17
#define MISC_PMD_INT_STATUS_pmd_chip_los_detect_int_DEFAULT        0

/* MISC :: PMD_INT_STATUS :: pmd_lia_rssi_valid_int [16:16] */
#define MISC_PMD_INT_STATUS_pmd_lia_rssi_valid_int_MASK            0x00010000
#define MISC_PMD_INT_STATUS_pmd_lia_rssi_valid_int_ALIGN           0
#define MISC_PMD_INT_STATUS_pmd_lia_rssi_valid_int_BITS            1
#define MISC_PMD_INT_STATUS_pmd_lia_rssi_valid_int_SHIFT           16
#define MISC_PMD_INT_STATUS_pmd_lia_rssi_valid_int_DEFAULT         0

/* MISC :: PMD_INT_STATUS :: pmd_lia_sd_detect_int [15:15] */
#define MISC_PMD_INT_STATUS_pmd_lia_sd_detect_int_MASK             0x00008000
#define MISC_PMD_INT_STATUS_pmd_lia_sd_detect_int_ALIGN            0
#define MISC_PMD_INT_STATUS_pmd_lia_sd_detect_int_BITS             1
#define MISC_PMD_INT_STATUS_pmd_lia_sd_detect_int_SHIFT            15
#define MISC_PMD_INT_STATUS_pmd_lia_sd_detect_int_DEFAULT          0

/* MISC :: PMD_INT_STATUS :: pmd_lia_los_detect_int [14:14] */
#define MISC_PMD_INT_STATUS_pmd_lia_los_detect_int_MASK            0x00004000
#define MISC_PMD_INT_STATUS_pmd_lia_los_detect_int_ALIGN           0
#define MISC_PMD_INT_STATUS_pmd_lia_los_detect_int_BITS            1
#define MISC_PMD_INT_STATUS_pmd_lia_los_detect_int_SHIFT           14
#define MISC_PMD_INT_STATUS_pmd_lia_los_detect_int_DEFAULT         0

/* MISC :: PMD_INT_STATUS :: pmd_adf_stat_rssi_valid_int [13:13] */
#define MISC_PMD_INT_STATUS_pmd_adf_stat_rssi_valid_int_MASK       0x00002000
#define MISC_PMD_INT_STATUS_pmd_adf_stat_rssi_valid_int_ALIGN      0
#define MISC_PMD_INT_STATUS_pmd_adf_stat_rssi_valid_int_BITS       1
#define MISC_PMD_INT_STATUS_pmd_adf_stat_rssi_valid_int_SHIFT      13
#define MISC_PMD_INT_STATUS_pmd_adf_stat_rssi_valid_int_DEFAULT    0

/* MISC :: PMD_INT_STATUS :: pmd_adf_pma_fifo_overrun_int [12:12] */
#define MISC_PMD_INT_STATUS_pmd_adf_pma_fifo_overrun_int_MASK      0x00001000
#define MISC_PMD_INT_STATUS_pmd_adf_pma_fifo_overrun_int_ALIGN     0
#define MISC_PMD_INT_STATUS_pmd_adf_pma_fifo_overrun_int_BITS      1
#define MISC_PMD_INT_STATUS_pmd_adf_pma_fifo_overrun_int_SHIFT     12
#define MISC_PMD_INT_STATUS_pmd_adf_pma_fifo_overrun_int_DEFAULT   0

/* MISC :: PMD_INT_STATUS :: pmd_adf_los_calc_overload_int [11:11] */
#define MISC_PMD_INT_STATUS_pmd_adf_los_calc_overload_int_MASK     0x00000800
#define MISC_PMD_INT_STATUS_pmd_adf_los_calc_overload_int_ALIGN    0
#define MISC_PMD_INT_STATUS_pmd_adf_los_calc_overload_int_BITS     1
#define MISC_PMD_INT_STATUS_pmd_adf_los_calc_overload_int_SHIFT    11
#define MISC_PMD_INT_STATUS_pmd_adf_los_calc_overload_int_DEFAULT  0

/* MISC :: PMD_INT_STATUS :: pmd_adf_sd_detect_int [10:10] */
#define MISC_PMD_INT_STATUS_pmd_adf_sd_detect_int_MASK             0x00000400
#define MISC_PMD_INT_STATUS_pmd_adf_sd_detect_int_ALIGN            0
#define MISC_PMD_INT_STATUS_pmd_adf_sd_detect_int_BITS             1
#define MISC_PMD_INT_STATUS_pmd_adf_sd_detect_int_SHIFT            10
#define MISC_PMD_INT_STATUS_pmd_adf_sd_detect_int_DEFAULT          0

/* MISC :: PMD_INT_STATUS :: pmd_adf_los_detect_int [09:09] */
#define MISC_PMD_INT_STATUS_pmd_adf_los_detect_int_MASK            0x00000200
#define MISC_PMD_INT_STATUS_pmd_adf_los_detect_int_ALIGN           0
#define MISC_PMD_INT_STATUS_pmd_adf_los_detect_int_BITS            1
#define MISC_PMD_INT_STATUS_pmd_adf_los_detect_int_SHIFT           9
#define MISC_PMD_INT_STATUS_pmd_adf_los_detect_int_DEFAULT         0

/* MISC :: PMD_INT_STATUS :: pmd_esc_int [08:08] */
#define MISC_PMD_INT_STATUS_pmd_esc_int_MASK                       0x00000100
#define MISC_PMD_INT_STATUS_pmd_esc_int_ALIGN                      0
#define MISC_PMD_INT_STATUS_pmd_esc_int_BITS                       1
#define MISC_PMD_INT_STATUS_pmd_esc_int_SHIFT                      8
#define MISC_PMD_INT_STATUS_pmd_esc_int_DEFAULT                    0

/* MISC :: PMD_INT_STATUS :: pmd_apd_fault_ovi_int [07:07] */
#define MISC_PMD_INT_STATUS_pmd_apd_fault_ovi_int_MASK             0x00000080
#define MISC_PMD_INT_STATUS_pmd_apd_fault_ovi_int_ALIGN            0
#define MISC_PMD_INT_STATUS_pmd_apd_fault_ovi_int_BITS             1
#define MISC_PMD_INT_STATUS_pmd_apd_fault_ovi_int_SHIFT            7
#define MISC_PMD_INT_STATUS_pmd_apd_fault_ovi_int_DEFAULT          0

/* MISC :: PMD_INT_STATUS :: pmd_ld_trig_matchfound1_int [06:06] */
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound1_int_MASK       0x00000040
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound1_int_ALIGN      0
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound1_int_BITS       1
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound1_int_SHIFT      6
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound1_int_DEFAULT    0

/* MISC :: PMD_INT_STATUS :: pmd_ld_trig_matchfound0_int [05:05] */
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound0_int_MASK       0x00000020
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound0_int_ALIGN      0
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound0_int_BITS       1
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound0_int_SHIFT      5
#define MISC_PMD_INT_STATUS_pmd_ld_trig_matchfound0_int_DEFAULT    0

/* MISC :: PMD_INT_STATUS :: pmd_ld_ewake_int [04:04] */
#define MISC_PMD_INT_STATUS_pmd_ld_ewake_int_MASK                  0x00000010
#define MISC_PMD_INT_STATUS_pmd_ld_ewake_int_ALIGN                 0
#define MISC_PMD_INT_STATUS_pmd_ld_ewake_int_BITS                  1
#define MISC_PMD_INT_STATUS_pmd_ld_ewake_int_SHIFT                 4
#define MISC_PMD_INT_STATUS_pmd_ld_ewake_int_DEFAULT               0

/* MISC :: PMD_INT_STATUS :: pmd_ld_bursten_int [03:03] */
#define MISC_PMD_INT_STATUS_pmd_ld_bursten_int_MASK                0x00000008
#define MISC_PMD_INT_STATUS_pmd_ld_bursten_int_ALIGN               0
#define MISC_PMD_INT_STATUS_pmd_ld_bursten_int_BITS                1
#define MISC_PMD_INT_STATUS_pmd_ld_bursten_int_SHIFT               3
#define MISC_PMD_INT_STATUS_pmd_ld_bursten_int_DEFAULT             0

/* MISC :: PMD_INT_STATUS :: pmd_clkrst_no_refclock_int [02:02] */
#define MISC_PMD_INT_STATUS_pmd_clkrst_no_refclock_int_MASK        0x00000004
#define MISC_PMD_INT_STATUS_pmd_clkrst_no_refclock_int_ALIGN       0
#define MISC_PMD_INT_STATUS_pmd_clkrst_no_refclock_int_BITS        1
#define MISC_PMD_INT_STATUS_pmd_clkrst_no_refclock_int_SHIFT       2
#define MISC_PMD_INT_STATUS_pmd_clkrst_no_refclock_int_DEFAULT     0

/* MISC :: PMD_INT_STATUS :: pmd_clkrst_pll_lost_lock_int [01:01] */
#define MISC_PMD_INT_STATUS_pmd_clkrst_pll_lost_lock_int_MASK      0x00000002
#define MISC_PMD_INT_STATUS_pmd_clkrst_pll_lost_lock_int_ALIGN     0
#define MISC_PMD_INT_STATUS_pmd_clkrst_pll_lost_lock_int_BITS      1
#define MISC_PMD_INT_STATUS_pmd_clkrst_pll_lost_lock_int_SHIFT     1
#define MISC_PMD_INT_STATUS_pmd_clkrst_pll_lost_lock_int_DEFAULT   0

/* MISC :: PMD_INT_STATUS :: pmd_sfr_watchdog_rst_int [00:00] */
#define MISC_PMD_INT_STATUS_pmd_sfr_watchdog_rst_int_MASK          0x00000001
#define MISC_PMD_INT_STATUS_pmd_sfr_watchdog_rst_int_ALIGN         0
#define MISC_PMD_INT_STATUS_pmd_sfr_watchdog_rst_int_BITS          1
#define MISC_PMD_INT_STATUS_pmd_sfr_watchdog_rst_int_SHIFT         0
#define MISC_PMD_INT_STATUS_pmd_sfr_watchdog_rst_int_DEFAULT       0

/***************************************************************************
 *PMD_INT_EN - Top-level interrupt enable
 ***************************************************************************/
/* MISC :: PMD_INT_EN :: reserved0 [31:30] */
#define MISC_PMD_INT_EN_reserved0_MASK                             0xc0000000
#define MISC_PMD_INT_EN_reserved0_ALIGN                            0
#define MISC_PMD_INT_EN_reserved0_BITS                             2
#define MISC_PMD_INT_EN_reserved0_SHIFT                            30

/* MISC :: PMD_INT_EN :: pmd_tmon_int_en [29:29] */
#define MISC_PMD_INT_EN_pmd_tmon_int_en_MASK                       0x20000000
#define MISC_PMD_INT_EN_pmd_tmon_int_en_ALIGN                      0
#define MISC_PMD_INT_EN_pmd_tmon_int_en_BITS                       1
#define MISC_PMD_INT_EN_pmd_tmon_int_en_SHIFT                      29
#define MISC_PMD_INT_EN_pmd_tmon_int_en_DEFAULT                    0

/* MISC :: PMD_INT_EN :: pmd_rxsd_debug_out_int_en [28:28] */
#define MISC_PMD_INT_EN_pmd_rxsd_debug_out_int_en_MASK             0x10000000
#define MISC_PMD_INT_EN_pmd_rxsd_debug_out_int_en_ALIGN            0
#define MISC_PMD_INT_EN_pmd_rxsd_debug_out_int_en_BITS             1
#define MISC_PMD_INT_EN_pmd_rxsd_debug_out_int_en_SHIFT            28
#define MISC_PMD_INT_EN_pmd_rxsd_debug_out_int_en_DEFAULT          0

/* MISC :: PMD_INT_EN :: pmd_pm_debug_out_int_en [27:27] */
#define MISC_PMD_INT_EN_pmd_pm_debug_out_int_en_MASK               0x08000000
#define MISC_PMD_INT_EN_pmd_pm_debug_out_int_en_ALIGN              0
#define MISC_PMD_INT_EN_pmd_pm_debug_out_int_en_BITS               1
#define MISC_PMD_INT_EN_pmd_pm_debug_out_int_en_SHIFT              27
#define MISC_PMD_INT_EN_pmd_pm_debug_out_int_en_DEFAULT            0

/* MISC :: PMD_INT_EN :: pmd_ldc_acl_mod_unlocked_int_en [26:26] */
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_unlocked_int_en_MASK       0x04000000
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_unlocked_int_en_ALIGN      0
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_unlocked_int_en_BITS       1
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_unlocked_int_en_SHIFT      26
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_unlocked_int_en_DEFAULT    0

/* MISC :: PMD_INT_EN :: pmd_ldc_acl_mod_locked_int_en [25:25] */
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_locked_int_en_MASK         0x02000000
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_locked_int_en_ALIGN        0
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_locked_int_en_BITS         1
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_locked_int_en_SHIFT        25
#define MISC_PMD_INT_EN_pmd_ldc_acl_mod_locked_int_en_DEFAULT      0

/* MISC :: PMD_INT_EN :: pmd_ldc_acl_bias_unlocked_int_en [24:24] */
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_unlocked_int_en_MASK      0x01000000
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_unlocked_int_en_ALIGN     0
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_unlocked_int_en_BITS      1
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_unlocked_int_en_SHIFT     24
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_unlocked_int_en_DEFAULT   0

/* MISC :: PMD_INT_EN :: pmd_ldc_acl_bias_locked_int_en [23:23] */
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_locked_int_en_MASK        0x00800000
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_locked_int_en_ALIGN       0
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_locked_int_en_BITS        1
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_locked_int_en_SHIFT       23
#define MISC_PMD_INT_EN_pmd_ldc_acl_bias_locked_int_en_DEFAULT     0

/* MISC :: PMD_INT_EN :: pmd_rxsd_value_int_en [22:22] */
#define MISC_PMD_INT_EN_pmd_rxsd_value_int_en_MASK                 0x00400000
#define MISC_PMD_INT_EN_pmd_rxsd_value_int_en_ALIGN                0
#define MISC_PMD_INT_EN_pmd_rxsd_value_int_en_BITS                 1
#define MISC_PMD_INT_EN_pmd_rxsd_value_int_en_SHIFT                22
#define MISC_PMD_INT_EN_pmd_rxsd_value_int_en_DEFAULT              0

/* MISC :: PMD_INT_EN :: pmd_pm_value_int_en [21:21] */
#define MISC_PMD_INT_EN_pmd_pm_value_int_en_MASK                   0x00200000
#define MISC_PMD_INT_EN_pmd_pm_value_int_en_ALIGN                  0
#define MISC_PMD_INT_EN_pmd_pm_value_int_en_BITS                   1
#define MISC_PMD_INT_EN_pmd_pm_value_int_en_SHIFT                  21
#define MISC_PMD_INT_EN_pmd_pm_value_int_en_DEFAULT                0

/* MISC :: PMD_INT_EN :: pmd_chip_sd_int_en [20:20] */
#define MISC_PMD_INT_EN_pmd_chip_sd_int_en_MASK                    0x00100000
#define MISC_PMD_INT_EN_pmd_chip_sd_int_en_ALIGN                   0
#define MISC_PMD_INT_EN_pmd_chip_sd_int_en_BITS                    1
#define MISC_PMD_INT_EN_pmd_chip_sd_int_en_SHIFT                   20
#define MISC_PMD_INT_EN_pmd_chip_sd_int_en_DEFAULT                 0

/* MISC :: PMD_INT_EN :: pmd_ldc_stat_done_int_en [19:19] */
#define MISC_PMD_INT_EN_pmd_ldc_stat_done_int_en_MASK              0x00080000
#define MISC_PMD_INT_EN_pmd_ldc_stat_done_int_en_ALIGN             0
#define MISC_PMD_INT_EN_pmd_ldc_stat_done_int_en_BITS              1
#define MISC_PMD_INT_EN_pmd_ldc_stat_done_int_en_SHIFT             19
#define MISC_PMD_INT_EN_pmd_ldc_stat_done_int_en_DEFAULT           0

/* MISC :: PMD_INT_EN :: pmd_chip_mb_written_int_en [18:18] */
#define MISC_PMD_INT_EN_pmd_chip_mb_written_int_en_MASK            0x00040000
#define MISC_PMD_INT_EN_pmd_chip_mb_written_int_en_ALIGN           0
#define MISC_PMD_INT_EN_pmd_chip_mb_written_int_en_BITS            1
#define MISC_PMD_INT_EN_pmd_chip_mb_written_int_en_SHIFT           18
#define MISC_PMD_INT_EN_pmd_chip_mb_written_int_en_DEFAULT         0

/* MISC :: PMD_INT_EN :: pmd_chip_los_detect_int_en [17:17] */
#define MISC_PMD_INT_EN_pmd_chip_los_detect_int_en_MASK            0x00020000
#define MISC_PMD_INT_EN_pmd_chip_los_detect_int_en_ALIGN           0
#define MISC_PMD_INT_EN_pmd_chip_los_detect_int_en_BITS            1
#define MISC_PMD_INT_EN_pmd_chip_los_detect_int_en_SHIFT           17
#define MISC_PMD_INT_EN_pmd_chip_los_detect_int_en_DEFAULT         0

/* MISC :: PMD_INT_EN :: pmd_lia_rssi_valid_int_en [16:16] */
#define MISC_PMD_INT_EN_pmd_lia_rssi_valid_int_en_MASK             0x00010000
#define MISC_PMD_INT_EN_pmd_lia_rssi_valid_int_en_ALIGN            0
#define MISC_PMD_INT_EN_pmd_lia_rssi_valid_int_en_BITS             1
#define MISC_PMD_INT_EN_pmd_lia_rssi_valid_int_en_SHIFT            16
#define MISC_PMD_INT_EN_pmd_lia_rssi_valid_int_en_DEFAULT          0

/* MISC :: PMD_INT_EN :: pmd_lia_sd_detect_int_en [15:15] */
#define MISC_PMD_INT_EN_pmd_lia_sd_detect_int_en_MASK              0x00008000
#define MISC_PMD_INT_EN_pmd_lia_sd_detect_int_en_ALIGN             0
#define MISC_PMD_INT_EN_pmd_lia_sd_detect_int_en_BITS              1
#define MISC_PMD_INT_EN_pmd_lia_sd_detect_int_en_SHIFT             15
#define MISC_PMD_INT_EN_pmd_lia_sd_detect_int_en_DEFAULT           0

/* MISC :: PMD_INT_EN :: pmd_lia_los_detect_int_en [14:14] */
#define MISC_PMD_INT_EN_pmd_lia_los_detect_int_en_MASK             0x00004000
#define MISC_PMD_INT_EN_pmd_lia_los_detect_int_en_ALIGN            0
#define MISC_PMD_INT_EN_pmd_lia_los_detect_int_en_BITS             1
#define MISC_PMD_INT_EN_pmd_lia_los_detect_int_en_SHIFT            14
#define MISC_PMD_INT_EN_pmd_lia_los_detect_int_en_DEFAULT          0

/* MISC :: PMD_INT_EN :: pmd_adf_stat_rssi_valid_int_en [13:13] */
#define MISC_PMD_INT_EN_pmd_adf_stat_rssi_valid_int_en_MASK        0x00002000
#define MISC_PMD_INT_EN_pmd_adf_stat_rssi_valid_int_en_ALIGN       0
#define MISC_PMD_INT_EN_pmd_adf_stat_rssi_valid_int_en_BITS        1
#define MISC_PMD_INT_EN_pmd_adf_stat_rssi_valid_int_en_SHIFT       13
#define MISC_PMD_INT_EN_pmd_adf_stat_rssi_valid_int_en_DEFAULT     0

/* MISC :: PMD_INT_EN :: pmd_adf_pma_fifo_overrun_int_en [12:12] */
#define MISC_PMD_INT_EN_pmd_adf_pma_fifo_overrun_int_en_MASK       0x00001000
#define MISC_PMD_INT_EN_pmd_adf_pma_fifo_overrun_int_en_ALIGN      0
#define MISC_PMD_INT_EN_pmd_adf_pma_fifo_overrun_int_en_BITS       1
#define MISC_PMD_INT_EN_pmd_adf_pma_fifo_overrun_int_en_SHIFT      12
#define MISC_PMD_INT_EN_pmd_adf_pma_fifo_overrun_int_en_DEFAULT    0

/* MISC :: PMD_INT_EN :: pmd_adf_los_calc_overload_int_en [11:11] */
#define MISC_PMD_INT_EN_pmd_adf_los_calc_overload_int_en_MASK      0x00000800
#define MISC_PMD_INT_EN_pmd_adf_los_calc_overload_int_en_ALIGN     0
#define MISC_PMD_INT_EN_pmd_adf_los_calc_overload_int_en_BITS      1
#define MISC_PMD_INT_EN_pmd_adf_los_calc_overload_int_en_SHIFT     11
#define MISC_PMD_INT_EN_pmd_adf_los_calc_overload_int_en_DEFAULT   0

/* MISC :: PMD_INT_EN :: pmd_adf_sd_detect_int_en [10:10] */
#define MISC_PMD_INT_EN_pmd_adf_sd_detect_int_en_MASK              0x00000400
#define MISC_PMD_INT_EN_pmd_adf_sd_detect_int_en_ALIGN             0
#define MISC_PMD_INT_EN_pmd_adf_sd_detect_int_en_BITS              1
#define MISC_PMD_INT_EN_pmd_adf_sd_detect_int_en_SHIFT             10
#define MISC_PMD_INT_EN_pmd_adf_sd_detect_int_en_DEFAULT           0

/* MISC :: PMD_INT_EN :: pmd_adf_los_alarm_int_en [09:09] */
#define MISC_PMD_INT_EN_pmd_adf_los_alarm_int_en_MASK              0x00000200
#define MISC_PMD_INT_EN_pmd_adf_los_alarm_int_en_ALIGN             0
#define MISC_PMD_INT_EN_pmd_adf_los_alarm_int_en_BITS              1
#define MISC_PMD_INT_EN_pmd_adf_los_alarm_int_en_SHIFT             9
#define MISC_PMD_INT_EN_pmd_adf_los_alarm_int_en_DEFAULT           0

/* MISC :: PMD_INT_EN :: pmd_esc_int_en [08:08] */
#define MISC_PMD_INT_EN_pmd_esc_int_en_MASK                        0x00000100
#define MISC_PMD_INT_EN_pmd_esc_int_en_ALIGN                       0
#define MISC_PMD_INT_EN_pmd_esc_int_en_BITS                        1
#define MISC_PMD_INT_EN_pmd_esc_int_en_SHIFT                       8
#define MISC_PMD_INT_EN_pmd_esc_int_en_DEFAULT                     0

/* MISC :: PMD_INT_EN :: pmd_apd_fault_ovi_int_en [07:07] */
#define MISC_PMD_INT_EN_pmd_apd_fault_ovi_int_en_MASK              0x00000080
#define MISC_PMD_INT_EN_pmd_apd_fault_ovi_int_en_ALIGN             0
#define MISC_PMD_INT_EN_pmd_apd_fault_ovi_int_en_BITS              1
#define MISC_PMD_INT_EN_pmd_apd_fault_ovi_int_en_SHIFT             7
#define MISC_PMD_INT_EN_pmd_apd_fault_ovi_int_en_DEFAULT           0

/* MISC :: PMD_INT_EN :: pmd_ld_trig_matchfound1_int_en [06:06] */
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound1_int_en_MASK        0x00000040
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound1_int_en_ALIGN       0
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound1_int_en_BITS        1
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound1_int_en_SHIFT       6
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound1_int_en_DEFAULT     0

/* MISC :: PMD_INT_EN :: pmd_ld_trig_matchfound0_int_en [05:05] */
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound0_int_en_MASK        0x00000020
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound0_int_en_ALIGN       0
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound0_int_en_BITS        1
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound0_int_en_SHIFT       5
#define MISC_PMD_INT_EN_pmd_ld_trig_matchfound0_int_en_DEFAULT     0

/* MISC :: PMD_INT_EN :: pmd_ld_ewake_int_en [04:04] */
#define MISC_PMD_INT_EN_pmd_ld_ewake_int_en_MASK                   0x00000010
#define MISC_PMD_INT_EN_pmd_ld_ewake_int_en_ALIGN                  0
#define MISC_PMD_INT_EN_pmd_ld_ewake_int_en_BITS                   1
#define MISC_PMD_INT_EN_pmd_ld_ewake_int_en_SHIFT                  4
#define MISC_PMD_INT_EN_pmd_ld_ewake_int_en_DEFAULT                0

/* MISC :: PMD_INT_EN :: pmd_ld_bursten_int_en [03:03] */
#define MISC_PMD_INT_EN_pmd_ld_bursten_int_en_MASK                 0x00000008
#define MISC_PMD_INT_EN_pmd_ld_bursten_int_en_ALIGN                0
#define MISC_PMD_INT_EN_pmd_ld_bursten_int_en_BITS                 1
#define MISC_PMD_INT_EN_pmd_ld_bursten_int_en_SHIFT                3
#define MISC_PMD_INT_EN_pmd_ld_bursten_int_en_DEFAULT              0

/* MISC :: PMD_INT_EN :: pmd_clkrst_no_refclock_int_en [02:02] */
#define MISC_PMD_INT_EN_pmd_clkrst_no_refclock_int_en_MASK         0x00000004
#define MISC_PMD_INT_EN_pmd_clkrst_no_refclock_int_en_ALIGN        0
#define MISC_PMD_INT_EN_pmd_clkrst_no_refclock_int_en_BITS         1
#define MISC_PMD_INT_EN_pmd_clkrst_no_refclock_int_en_SHIFT        2
#define MISC_PMD_INT_EN_pmd_clkrst_no_refclock_int_en_DEFAULT      0

/* MISC :: PMD_INT_EN :: pmd_clkrst_pll_lost_lock_int_en [01:01] */
#define MISC_PMD_INT_EN_pmd_clkrst_pll_lost_lock_int_en_MASK       0x00000002
#define MISC_PMD_INT_EN_pmd_clkrst_pll_lost_lock_int_en_ALIGN      0
#define MISC_PMD_INT_EN_pmd_clkrst_pll_lost_lock_int_en_BITS       1
#define MISC_PMD_INT_EN_pmd_clkrst_pll_lost_lock_int_en_SHIFT      1
#define MISC_PMD_INT_EN_pmd_clkrst_pll_lost_lock_int_en_DEFAULT    0

/* MISC :: PMD_INT_EN :: pmd_sfr_watchdog_rst_int_en [00:00] */
#define MISC_PMD_INT_EN_pmd_sfr_watchdog_rst_int_en_MASK           0x00000001
#define MISC_PMD_INT_EN_pmd_sfr_watchdog_rst_int_en_ALIGN          0
#define MISC_PMD_INT_EN_pmd_sfr_watchdog_rst_int_en_BITS           1
#define MISC_PMD_INT_EN_pmd_sfr_watchdog_rst_int_en_SHIFT          0
#define MISC_PMD_INT_EN_pmd_sfr_watchdog_rst_int_en_DEFAULT        0

/***************************************************************************
 *CPU_RAM_PSM_VDD - PSM VDD control for CPU internal memory
 ***************************************************************************/
/* MISC :: CPU_RAM_PSM_VDD :: reserved0 [31:01] */
#define MISC_CPU_RAM_PSM_VDD_reserved0_MASK                        0xfffffffe
#define MISC_CPU_RAM_PSM_VDD_reserved0_ALIGN                       0
#define MISC_CPU_RAM_PSM_VDD_reserved0_BITS                        31
#define MISC_CPU_RAM_PSM_VDD_reserved0_SHIFT                       1

/* MISC :: CPU_RAM_PSM_VDD :: cpu_psm_vdd_enb [00:00] */
#define MISC_CPU_RAM_PSM_VDD_cpu_psm_vdd_enb_MASK                  0x00000001
#define MISC_CPU_RAM_PSM_VDD_cpu_psm_vdd_enb_ALIGN                 0
#define MISC_CPU_RAM_PSM_VDD_cpu_psm_vdd_enb_BITS                  1
#define MISC_CPU_RAM_PSM_VDD_cpu_psm_vdd_enb_SHIFT                 0
#define MISC_CPU_RAM_PSM_VDD_cpu_psm_vdd_enb_DEFAULT               0

/***************************************************************************
 *PMD_RAM_PSM_VDD - PSM VDD control for PMD main memory
 ***************************************************************************/
/* MISC :: PMD_RAM_PSM_VDD :: reserved0 [31:26] */
#define MISC_PMD_RAM_PSM_VDD_reserved0_MASK                        0xfc000000
#define MISC_PMD_RAM_PSM_VDD_reserved0_ALIGN                       0
#define MISC_PMD_RAM_PSM_VDD_reserved0_BITS                        6
#define MISC_PMD_RAM_PSM_VDD_reserved0_SHIFT                       26

/* MISC :: PMD_RAM_PSM_VDD :: pmd_data_ram_standby [25:25] */
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_standby_MASK             0x02000000
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_standby_ALIGN            0
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_standby_BITS             1
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_standby_SHIFT            25
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_standby_DEFAULT          0

/* MISC :: PMD_RAM_PSM_VDD :: pmd_data_ram_pda_enb [24:17] */
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_pda_enb_MASK             0x01fe0000
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_pda_enb_ALIGN            0
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_pda_enb_BITS             8
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_pda_enb_SHIFT            17
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_pda_enb_DEFAULT          0

/* MISC :: PMD_RAM_PSM_VDD :: pmd_data_ram_psm_vdd_enb [16:16] */
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_psm_vdd_enb_MASK         0x00010000
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_psm_vdd_enb_ALIGN        0
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_psm_vdd_enb_BITS         1
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_psm_vdd_enb_SHIFT        16
#define MISC_PMD_RAM_PSM_VDD_pmd_data_ram_psm_vdd_enb_DEFAULT      0

/* MISC :: PMD_RAM_PSM_VDD :: reserved1 [15:10] */
#define MISC_PMD_RAM_PSM_VDD_reserved1_MASK                        0x0000fc00
#define MISC_PMD_RAM_PSM_VDD_reserved1_ALIGN                       0
#define MISC_PMD_RAM_PSM_VDD_reserved1_BITS                        6
#define MISC_PMD_RAM_PSM_VDD_reserved1_SHIFT                       10

/* MISC :: PMD_RAM_PSM_VDD :: pmd_code_ram_standby [09:09] */
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_standby_MASK             0x00000200
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_standby_ALIGN            0
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_standby_BITS             1
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_standby_SHIFT            9
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_standby_DEFAULT          0

/* MISC :: PMD_RAM_PSM_VDD :: pmd_code_ram_pda_enb [08:01] */
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_pda_enb_MASK             0x000001fe
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_pda_enb_ALIGN            0
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_pda_enb_BITS             8
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_pda_enb_SHIFT            1
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_pda_enb_DEFAULT          0

/* MISC :: PMD_RAM_PSM_VDD :: pmd_code_ram_psm_vdd_enb [00:00] */
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_psm_vdd_enb_MASK         0x00000001
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_psm_vdd_enb_ALIGN        0
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_psm_vdd_enb_BITS         1
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_psm_vdd_enb_SHIFT        0
#define MISC_PMD_RAM_PSM_VDD_pmd_code_ram_psm_vdd_enb_DEFAULT      0

/***************************************************************************
 *MISC_STAT_BURST_COUNT - The number of BE rising edges since statistic was reset
 ***************************************************************************/
/* MISC :: MISC_STAT_BURST_COUNT :: MISC_STAT_BURST_COUNT_ENABLE [31:31] */
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_ENABLE_MASK 0x80000000
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_ENABLE_ALIGN 0
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_ENABLE_BITS 1
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_ENABLE_SHIFT 31
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_ENABLE_DEFAULT 0

/* MISC :: MISC_STAT_BURST_COUNT :: MISC_STAT_BURST_COUNT [30:00] */
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_MASK      0x7fffffff
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_ALIGN     0
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_BITS      31
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_SHIFT     0
#define MISC_MISC_STAT_BURST_COUNT_MISC_STAT_BURST_COUNT_DEFAULT   0

/***************************************************************************
 *MISC_STAT_BURST_ON_TIME - The total number of system clocks BE was high
 ***************************************************************************/
/* MISC :: MISC_STAT_BURST_ON_TIME :: MISC_STAT_BURST_ON_TIME_ENABLE [31:31] */
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_ENABLE_MASK 0x80000000
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_ENABLE_ALIGN 0
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_ENABLE_BITS 1
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_ENABLE_SHIFT 31
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_ENABLE_DEFAULT 0

/* MISC :: MISC_STAT_BURST_ON_TIME :: MISC_STAT_BURST_ON_TIME [30:00] */
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_MASK  0x7fffffff
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_ALIGN 0
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_BITS  31
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_SHIFT 0
#define MISC_MISC_STAT_BURST_ON_TIME_MISC_STAT_BURST_ON_TIME_DEFAULT 0

/***************************************************************************
 *MISC_STAT_BURST_ON_MAX - The maximum number of system clocks a single BE was high
 ***************************************************************************/
/* MISC :: MISC_STAT_BURST_ON_MAX :: MISC_STAT_BURST_ON_MAX_ENABLE [31:31] */
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_ENABLE_MASK 0x80000000
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_ENABLE_ALIGN 0
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_ENABLE_BITS 1
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_ENABLE_SHIFT 31
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_ENABLE_DEFAULT 0

/* MISC :: MISC_STAT_BURST_ON_MAX :: MISC_STAT_BURST_ON_MAX [30:00] */
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_MASK    0x7fffffff
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_ALIGN   0
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_BITS    31
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_SHIFT   0
#define MISC_MISC_STAT_BURST_ON_MAX_MISC_STAT_BURST_ON_MAX_DEFAULT 0

/***************************************************************************
 *MISC_STAT_BURST_ON_MIN - The minimum number of system clocks a single BE was high
 ***************************************************************************/
/* MISC :: MISC_STAT_BURST_ON_MIN :: MISC_STAT_BURST_ON_MIN_ENABLE [31:31] */
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_ENABLE_MASK 0x80000000
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_ENABLE_ALIGN 0
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_ENABLE_BITS 1
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_ENABLE_SHIFT 31
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_ENABLE_DEFAULT 0

/* MISC :: MISC_STAT_BURST_ON_MIN :: MISC_STAT_BURST_ON_MIN [30:00] */
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_MASK    0x7fffffff
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_ALIGN   0
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_BITS    31
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_SHIFT   0
#define MISC_MISC_STAT_BURST_ON_MIN_MISC_STAT_BURST_ON_MIN_DEFAULT 2147483647

/***************************************************************************
 *MISC_STAT_BURST_OFF_TIME - The total number of system clocks BE was low
 ***************************************************************************/
/* MISC :: MISC_STAT_BURST_OFF_TIME :: MISC_STAT_BURST_OFF_TIME_ENABLE [31:31] */
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_ENABLE_MASK 0x80000000
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_ENABLE_ALIGN 0
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_ENABLE_BITS 1
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_ENABLE_SHIFT 31
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_ENABLE_DEFAULT 0

/* MISC :: MISC_STAT_BURST_OFF_TIME :: MISC_STAT_BURST_OFF_TIME [30:00] */
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_MASK 0x7fffffff
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_ALIGN 0
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_BITS 31
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_SHIFT 0
#define MISC_MISC_STAT_BURST_OFF_TIME_MISC_STAT_BURST_OFF_TIME_DEFAULT 0

/***************************************************************************
 *MISC_STAT_BURST_OFF_MAX - The maximum number of system clocks a single BE was low
 ***************************************************************************/
/* MISC :: MISC_STAT_BURST_OFF_MAX :: MISC_STAT_BURST_OFF_MAX_ENABLE [31:31] */
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_ENABLE_MASK 0x80000000
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_ENABLE_ALIGN 0
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_ENABLE_BITS 1
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_ENABLE_SHIFT 31
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_ENABLE_DEFAULT 0

/* MISC :: MISC_STAT_BURST_OFF_MAX :: MISC_STAT_BURST_OFF_MAX [30:00] */
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_MASK  0x7fffffff
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_ALIGN 0
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_BITS  31
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_SHIFT 0
#define MISC_MISC_STAT_BURST_OFF_MAX_MISC_STAT_BURST_OFF_MAX_DEFAULT 0

/***************************************************************************
 *MISC_STAT_BURST_OFF_MIN - The minimum number of system clocks a single BE was low
 ***************************************************************************/
/* MISC :: MISC_STAT_BURST_OFF_MIN :: MISC_STAT_BURST_OFF_MIN_ENABLE [31:31] */
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_ENABLE_MASK 0x80000000
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_ENABLE_ALIGN 0
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_ENABLE_BITS 1
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_ENABLE_SHIFT 31
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_ENABLE_DEFAULT 0

/* MISC :: MISC_STAT_BURST_OFF_MIN :: MISC_STAT_BURST_OFF_MIN [30:00] */
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_MASK  0x7fffffff
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_ALIGN 0
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_BITS  31
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_SHIFT 0
#define MISC_MISC_STAT_BURST_OFF_MIN_MISC_STAT_BURST_OFF_MIN_DEFAULT 2147483647

#endif /* #ifndef MISC_H__ */

/* End of File */
