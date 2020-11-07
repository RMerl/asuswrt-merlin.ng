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

#ifndef LRL_H__
#define LRL_H__

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
#define LRL_LRL_CONTROL                          0x00000000 /* LRL configuration */
#define LRL_LRL_TIMES                            0x00000004 /* LRL timer configuration */
#define LRL_LRL_RSSI_PEAKPOS_THRESH              0x00000008 /* LRL peak positive LOS threshold */
#define LRL_LRL_RSSI_PEAKNEG_THRESH              0x0000000c /* LRL peak negative LOS threshold */
#define LRL_LRL_RSSI_PEAKPOS                     0x00000010 /* LRL peak positive RSSI value */
#define LRL_LRL_RSSI_PEAKNEG                     0x00000014 /* LRL peak negative RSSI value */
#define LRL_LRL_PARAM                            0x00000018 /* LRL Parameter Register */
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY           0x0000001c /* LRL LIA LOS debounce delay */

/***************************************************************************
 *LRL_CONTROL - LRL configuration
 ***************************************************************************/
/* LRL :: LRL_CONTROL :: reserved0 [31:10] */
#define LRL_LRL_CONTROL_reserved0_MASK                             0xfffffc00
#define LRL_LRL_CONTROL_reserved0_ALIGN                            0
#define LRL_LRL_CONTROL_reserved0_BITS                             22
#define LRL_LRL_CONTROL_reserved0_SHIFT                            10

/* LRL :: LRL_CONTROL :: lrl_diff_los_mode [09:09] */
#define LRL_LRL_CONTROL_lrl_diff_los_mode_MASK                     0x00000200
#define LRL_LRL_CONTROL_lrl_diff_los_mode_ALIGN                    0
#define LRL_LRL_CONTROL_lrl_diff_los_mode_BITS                     1
#define LRL_LRL_CONTROL_lrl_diff_los_mode_SHIFT                    9
#define LRL_LRL_CONTROL_lrl_diff_los_mode_DEFAULT                  0

/* LRL :: LRL_CONTROL :: lrl_adf_los_cpu_ctl_state [08:08] */
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_state_MASK             0x00000100
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_state_ALIGN            0
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_state_BITS             1
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_state_SHIFT            8
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_state_DEFAULT          0

/* LRL :: LRL_CONTROL :: lrl_adf_los_cpu_ctl_en [07:07] */
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_en_MASK                0x00000080
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_en_ALIGN               0
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_en_BITS                1
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_en_SHIFT               7
#define LRL_LRL_CONTROL_lrl_adf_los_cpu_ctl_en_DEFAULT             0

/* LRL :: LRL_CONTROL :: lrl_adf_sd_cpu_ctl_state [06:06] */
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_state_MASK              0x00000040
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_state_ALIGN             0
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_state_BITS              1
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_state_SHIFT             6
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_state_DEFAULT           0

/* LRL :: LRL_CONTROL :: lrl_adf_sd_cpu_ctl_en [05:05] */
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_en_MASK                 0x00000020
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_en_ALIGN                0
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_en_BITS                 1
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_en_SHIFT                5
#define LRL_LRL_CONTROL_lrl_adf_sd_cpu_ctl_en_DEFAULT              0

/* LRL :: LRL_CONTROL :: lrl_lia_los_cpu_ctl_state [04:04] */
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_state_MASK             0x00000010
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_state_ALIGN            0
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_state_BITS             1
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_state_SHIFT            4
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_state_DEFAULT          0

/* LRL :: LRL_CONTROL :: lrl_lia_los_cpu_ctl_en [03:03] */
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_en_MASK                0x00000008
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_en_ALIGN               0
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_en_BITS                1
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_en_SHIFT               3
#define LRL_LRL_CONTROL_lrl_lia_los_cpu_ctl_en_DEFAULT             0

/* LRL :: LRL_CONTROL :: lrl_lia_sd_cpu_ctl_state [02:02] */
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_state_MASK              0x00000004
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_state_ALIGN             0
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_state_BITS              1
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_state_SHIFT             2
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_state_DEFAULT           0

/* LRL :: LRL_CONTROL :: lrl_lia_sd_cpu_ctl_en [01:01] */
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_en_MASK                 0x00000002
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_en_ALIGN                0
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_en_BITS                 1
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_en_SHIFT                1
#define LRL_LRL_CONTROL_lrl_lia_sd_cpu_ctl_en_DEFAULT              0

/* LRL :: LRL_CONTROL :: lrl_en [00:00] */
#define LRL_LRL_CONTROL_lrl_en_MASK                                0x00000001
#define LRL_LRL_CONTROL_lrl_en_ALIGN                               0
#define LRL_LRL_CONTROL_lrl_en_BITS                                1
#define LRL_LRL_CONTROL_lrl_en_SHIFT                               0
#define LRL_LRL_CONTROL_lrl_en_DEFAULT                             0

/***************************************************************************
 *LRL_TIMES - LRL timer configuration
 ***************************************************************************/
/* LRL :: LRL_TIMES :: reserved0 [31:22] */
#define LRL_LRL_TIMES_reserved0_MASK                               0xffc00000
#define LRL_LRL_TIMES_reserved0_ALIGN                              0
#define LRL_LRL_TIMES_reserved0_BITS                               10
#define LRL_LRL_TIMES_reserved0_SHIFT                              22

/* LRL :: LRL_TIMES :: lrl_pls_width [21:16] */
#define LRL_LRL_TIMES_lrl_pls_width_MASK                           0x003f0000
#define LRL_LRL_TIMES_lrl_pls_width_ALIGN                          0
#define LRL_LRL_TIMES_lrl_pls_width_BITS                           6
#define LRL_LRL_TIMES_lrl_pls_width_SHIFT                          16
#define LRL_LRL_TIMES_lrl_pls_width_DEFAULT                        7

/* LRL :: LRL_TIMES :: reserved1 [15:11] */
#define LRL_LRL_TIMES_reserved1_MASK                               0x0000f800
#define LRL_LRL_TIMES_reserved1_ALIGN                              0
#define LRL_LRL_TIMES_reserved1_BITS                               5
#define LRL_LRL_TIMES_reserved1_SHIFT                              11

/* LRL :: LRL_TIMES :: lrl_readpk_wait [10:00] */
#define LRL_LRL_TIMES_lrl_readpk_wait_MASK                         0x000007ff
#define LRL_LRL_TIMES_lrl_readpk_wait_ALIGN                        0
#define LRL_LRL_TIMES_lrl_readpk_wait_BITS                         11
#define LRL_LRL_TIMES_lrl_readpk_wait_SHIFT                        0
#define LRL_LRL_TIMES_lrl_readpk_wait_DEFAULT                      200

/***************************************************************************
 *LRL_RSSI_PEAKPOS_THRESH - LRL peak positive LOS threshold
 ***************************************************************************/
/* LRL :: LRL_RSSI_PEAKPOS_THRESH :: reserved0 [31:16] */
#define LRL_LRL_RSSI_PEAKPOS_THRESH_reserved0_MASK                 0xffff0000
#define LRL_LRL_RSSI_PEAKPOS_THRESH_reserved0_ALIGN                0
#define LRL_LRL_RSSI_PEAKPOS_THRESH_reserved0_BITS                 16
#define LRL_LRL_RSSI_PEAKPOS_THRESH_reserved0_SHIFT                16

/* LRL :: LRL_RSSI_PEAKPOS_THRESH :: lrl_rssi_peakpos_off_thresh [15:08] */
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_off_thresh_MASK 0x0000ff00
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_off_thresh_ALIGN 0
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_off_thresh_BITS 8
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_off_thresh_SHIFT 8
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_off_thresh_DEFAULT 0

/* LRL :: LRL_RSSI_PEAKPOS_THRESH :: lrl_rssi_peakpos_thresh [07:00] */
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_thresh_MASK   0x000000ff
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_thresh_ALIGN  0
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_thresh_BITS   8
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_thresh_SHIFT  0
#define LRL_LRL_RSSI_PEAKPOS_THRESH_lrl_rssi_peakpos_thresh_DEFAULT 0

/***************************************************************************
 *LRL_RSSI_PEAKNEG_THRESH - LRL peak negative LOS threshold
 ***************************************************************************/
/* LRL :: LRL_RSSI_PEAKNEG_THRESH :: reserved0 [31:16] */
#define LRL_LRL_RSSI_PEAKNEG_THRESH_reserved0_MASK                 0xffff0000
#define LRL_LRL_RSSI_PEAKNEG_THRESH_reserved0_ALIGN                0
#define LRL_LRL_RSSI_PEAKNEG_THRESH_reserved0_BITS                 16
#define LRL_LRL_RSSI_PEAKNEG_THRESH_reserved0_SHIFT                16

/* LRL :: LRL_RSSI_PEAKNEG_THRESH :: lrl_rssi_peakneg_off_thresh [15:08] */
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_off_thresh_MASK 0x0000ff00
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_off_thresh_ALIGN 0
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_off_thresh_BITS 8
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_off_thresh_SHIFT 8
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_off_thresh_DEFAULT 0

/* LRL :: LRL_RSSI_PEAKNEG_THRESH :: lrl_rssi_peakneg_thresh [07:00] */
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_thresh_MASK   0x000000ff
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_thresh_ALIGN  0
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_thresh_BITS   8
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_thresh_SHIFT  0
#define LRL_LRL_RSSI_PEAKNEG_THRESH_lrl_rssi_peakneg_thresh_DEFAULT 0

/***************************************************************************
 *LRL_RSSI_PEAKPOS - LRL peak positive RSSI value
 ***************************************************************************/
/* LRL :: LRL_RSSI_PEAKPOS :: reserved0 [31:08] */
#define LRL_LRL_RSSI_PEAKPOS_reserved0_MASK                        0xffffff00
#define LRL_LRL_RSSI_PEAKPOS_reserved0_ALIGN                       0
#define LRL_LRL_RSSI_PEAKPOS_reserved0_BITS                        24
#define LRL_LRL_RSSI_PEAKPOS_reserved0_SHIFT                       8

/* LRL :: LRL_RSSI_PEAKPOS :: lrl_rssi_peakpos [07:00] */
#define LRL_LRL_RSSI_PEAKPOS_lrl_rssi_peakpos_MASK                 0x000000ff
#define LRL_LRL_RSSI_PEAKPOS_lrl_rssi_peakpos_ALIGN                0
#define LRL_LRL_RSSI_PEAKPOS_lrl_rssi_peakpos_BITS                 8
#define LRL_LRL_RSSI_PEAKPOS_lrl_rssi_peakpos_SHIFT                0
#define LRL_LRL_RSSI_PEAKPOS_lrl_rssi_peakpos_DEFAULT              0

/***************************************************************************
 *LRL_RSSI_PEAKNEG - LRL peak negative RSSI value
 ***************************************************************************/
/* LRL :: LRL_RSSI_PEAKNEG :: reserved0 [31:08] */
#define LRL_LRL_RSSI_PEAKNEG_reserved0_MASK                        0xffffff00
#define LRL_LRL_RSSI_PEAKNEG_reserved0_ALIGN                       0
#define LRL_LRL_RSSI_PEAKNEG_reserved0_BITS                        24
#define LRL_LRL_RSSI_PEAKNEG_reserved0_SHIFT                       8

/* LRL :: LRL_RSSI_PEAKNEG :: lrl_rssi_peakneg [07:00] */
#define LRL_LRL_RSSI_PEAKNEG_lrl_rssi_peakneg_MASK                 0x000000ff
#define LRL_LRL_RSSI_PEAKNEG_lrl_rssi_peakneg_ALIGN                0
#define LRL_LRL_RSSI_PEAKNEG_lrl_rssi_peakneg_BITS                 8
#define LRL_LRL_RSSI_PEAKNEG_lrl_rssi_peakneg_SHIFT                0
#define LRL_LRL_RSSI_PEAKNEG_lrl_rssi_peakneg_DEFAULT              0

/***************************************************************************
 *LRL_PARAM - LRL Parameter Register
 ***************************************************************************/
/* LRL :: LRL_PARAM :: reserved_for_eco0 [31:00] */
#define LRL_LRL_PARAM_reserved_for_eco0_MASK                       0xffffffff
#define LRL_LRL_PARAM_reserved_for_eco0_ALIGN                      0
#define LRL_LRL_PARAM_reserved_for_eco0_BITS                       32
#define LRL_LRL_PARAM_reserved_for_eco0_SHIFT                      0
#define LRL_LRL_PARAM_reserved_for_eco0_DEFAULT                    0

/***************************************************************************
 *LRL_LIA_LOS_DEBOUNCE_DELAY - LRL LIA LOS debounce delay
 ***************************************************************************/
/* LRL :: LRL_LIA_LOS_DEBOUNCE_DELAY :: reserved0 [31:04] */
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_reserved0_MASK              0xfffffff0
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_reserved0_ALIGN             0
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_reserved0_BITS              28
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_reserved0_SHIFT             4

/* LRL :: LRL_LIA_LOS_DEBOUNCE_DELAY :: lrl_lia_los_debounce_delay [03:00] */
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_lrl_lia_los_debounce_delay_MASK 0x0000000f
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_lrl_lia_los_debounce_delay_ALIGN 0
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_lrl_lia_los_debounce_delay_BITS 4
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_lrl_lia_los_debounce_delay_SHIFT 0
#define LRL_LRL_LIA_LOS_DEBOUNCE_DELAY_lrl_lia_los_debounce_delay_DEFAULT 0

#endif /* #ifndef LRL_H__ */

/* End of File */
