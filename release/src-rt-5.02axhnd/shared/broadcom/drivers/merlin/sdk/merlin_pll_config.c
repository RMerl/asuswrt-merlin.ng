 /*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
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

/** @file merlin_pll_config.c
 * Merlin PLL Configuration
 */

#include "merlin_mptwo_enum.h"

static err_code_t merlin_mptwo_configure_pll_ctrl_bus(merlin_access_t *ma, enum merlin_mptwo_pll_enum pll_cfg);

static err_code_t _set_merlin_pll_mode1(merlin_access_t *ma);
static err_code_t _set_merlin_pll_mode2(merlin_access_t *ma);
static err_code_t _set_merlin_pll_mode3(merlin_access_t *ma);
static err_code_t _set_merlin_pll_mode4(merlin_access_t *ma);

err_code_t merlin_mptwo_configure_pll(merlin_access_t *ma, enum merlin_mptwo_pll_enum pll_cfg) {
    uint8_t reset_state;

 /* Use this to restore defaults if reprogramming the PLL under dp-reset (typically Auto-Neg FW) */
 /* EFUN(wrc_ams_pll_div                      (  0x14)); */
 /* EFUN(wrc_ams_pll_en_8p5g                  (   0x0)); */
 /* EFUN(wrc_ams_pll_en_8p5g_vco              (   0x0)); */
 /* EFUN(wrc_ams_pll_i_ndiv_frac_h            (   0x0)); */
 /* EFUN(wrc_ams_pll_i_ndiv_frac_l            (   0x0)); */
 /* EFUN(wrc_ams_pll_i_ndiv_int               (  0xA8)); */
 /* EFUN(wrc_ams_pll_i_pll_frac_mode          (   0x0)); */
 /* EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb         (   0x1)); */
 /* EFUN(wrc_ams_pll_mmd_div_range            (   0x1)); */
 /* EFUN(wrc_ams_pll_mmd_en                   (   0x0)); */
 /* EFUN(wrc_ams_pll_mmd_prsc4or5pwdb         (   0x0)); */
 /* EFUN(wrc_ams_pll_mmd_prsc8or9pwdb         (   0x1)); */
 /* EFUN(wrc_ams_pll_mmd_resetb               (   0x1)); */

 /* Use core_s_rstb to re-initialize all registers to default before calling this function. */
    ESTM(reset_state = rdc_core_dp_reset_state());

    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: merlin_mptwo_configure_pll(..) called without core_dp_s_rstb=0\n"));
        return (_error(ERR_CODE_CORE_DP_NOT_RESET));
    }

    switch (pll_cfg) {

    /******************/
    /*  Integer Mode  */
    /******************/

    case MERLIN_MPTWO_pll_8p5GHz_106p25MHz:       /* i_pll_ctrl[63:59] div<4:0> =  1100 */
        EFUN(wrc_ams_pll_mmd_en               (   0x0));   /* 0x4000[13:13] */
        EFUN(wrc_ams_pll_en_8p5g              (   0x1));
        EFUN(wrc_ams_pll_en_8p5g_vco          (   0x1));
        EFUN(wrc_ams_pll_div                  (   0xC));   /* 0x6106[15:11] */
        break;

    case MERLIN_MPTWO_pll_10GHz_125MHz:           /* i_pll_ctrl[63:59] div<4:0> =  1100 */
        EFUN(wrc_ams_pll_mmd_en               (   0x0));   /* 0x4c58[13:13] */
        EFUN(wrc_ams_pll_div                  (   0xC));   /* 0x6106[15:11] */
        break;

    case MERLIN_MPTWO_pll_11p181GHz_174p703MHz:
    case MERLIN_MPTWO_pll_10GHz_156p25MHz:        /* i_pll_ctrl[63:59] div<4:0> =   100 */
        EFUN(wrc_ams_pll_mmd_en               (   0x0));   /* 0x4c44[13:13] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        break;

    case MERLIN_MPTWO_pll_10p3125GHz_156p25MHz:   /* i_pll_ctrl[63:59] div<4:0> = 10100 */
        EFUN(wrc_ams_pll_mmd_en               (   0x0));   /* 0x5036[13:13] */
        EFUN(wrc_ams_pll_div                  (  0x14));   /* 0xa106[15:11] */
        break;

    case MERLIN_MPTWO_pll_9p375GHz_156p25MHz:     /* i_pll_ctrl[63:59] div<4:0> = 11100 */
        EFUN(wrc_ams_pll_mmd_en               (   0x0));   /* 0x4c46[13:13] */
        EFUN(wrc_ams_pll_div                  (  0x1C));   /* 0xe106[15:11] */
        break;


    /* This mode has higher jitter than the          */
    /* corresponding fracn mode and has been removed */

 /* case MERLIN_MPTWO_pll_10GHz_50MHz:          *//* i_pll_ctrl[63:59] div<4:0> =    10 */
 /*     EFUN(wrc_ams_pll_mmd_en               (   0x0));     0x4000[13:13] */
 /*     EFUN(wrc_ams_pll_div                  (   0x2));     0x1106[15:11] */
 /*     break;                                                    */

    case MERLIN_MPTWO_pll_9p375GHz_50MHz:         /* i_pll_ctrl[63:59] div<4:0> = 11010 */
        EFUN(wrc_ams_pll_mmd_en               (   0x0));   /* 0x4cce[13:13] */
        EFUN(wrc_ams_pll_div                  (  0x1A));   /* 0xd106[15:11] */
        break;

    case MERLIN_MPTWO_pll_10p3125GHz_161p132MHz:  /* i_pll_ctrl[63:59] div<4:0> = 001XX */
        EFUN(wrc_ams_pll_mmd_en               (   0x0));   /* 0x5036[13:13] */
        EFUN(wrc_ams_pll_div                  (  0x04));   /* 0xa106[15:11] */
        break;

    /***************/
    /*  Frac Mode  */
    /***************/

    case MERLIN_MPTWO_pll_10p3125GHz_125MHz:      /* Divider value 82.5      */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6d06[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6d06[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6d06[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6d06[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x52));   /* 0x6c52[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2000));   /* 0x2000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c52[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p709GHz_125MHz:       /* Divider value 85.672    */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c56[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c56[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c56[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c56[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x55));   /* 0x6c55[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2B02));   /* 0x2b02[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x1));   /* 0x1040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c55[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p7545GHz_125MHz:      /* Divider value 86.036    */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c59[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c59[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c59[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c59[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x56));   /* 0x6c56[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        ( 0x24D));   /* 0x024d[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xD));   /* 0xd040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c56[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p9375GHz_125MHz:      /* Divider value 87.5      */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c56[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c56[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c56[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c56[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x57));   /* 0x6c57[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2000));   /* 0x2000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c57[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p049GHz_125MHz:       /* Divider value 88.392    */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c55[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c55[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c55[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c55[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x58));   /* 0x6c58[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x1916));   /* 0x1916[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x8));   /* 0x8040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c58[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p095GHz_125MHz:       /* Divider value 88.76     */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6d49[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6d49[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6d49[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6d49[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x58));   /* 0x6c58[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x30A3));   /* 0x30a3[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xD));   /* 0xd040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c58[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p14273GHz_125MHz:     /* Divider value 89.14184  */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c59[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c59[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c59[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c59[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x59));   /* 0x6c59[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        ( 0x913));   /* 0x0913[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xF));   /* 0xf040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c59[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p181GHz_125MHz:       /* Divider value 89.448    */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c52[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c52[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c52[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c52[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x59));   /* 0x6c59[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x1CAC));   /* 0x1cac[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x1));   /* 0x1040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c59[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p25GHz_125MHz:        /* Divider value 90        */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c59[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c59[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c59[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c59[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x0));   /* 0x5200[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           ( 0x149));   /* 0x6d49[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (   0x0));   /* 0x0000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5201[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6d49[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p45863GHz_125MHz:     /* Divider value 91.66904  */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c52[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c52[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c52[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c52[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x5B));   /* 0x6c5b[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2AD1));   /* 0x2ad1[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x9));   /* 0x9040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c5b[14:14] */
        break;

    case MERLIN_MPTWO_pll_8p5GHz_125MHz:          /* Divider value 68        */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_en_8p5g              (   0x1));
        EFUN(wrc_ams_pll_en_8p5g_vco          (   0x1));
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c57[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c57[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c57[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c57[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x0));   /* 0x5200[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0xE4));   /* 0x6ce4[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (   0x0));   /* 0x0000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5201[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6ce4[14:14] */
        break;

    case MERLIN_MPTWO_pll_9p375GHz_125MHz:        /* Divider value 75        */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c58[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c58[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c58[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c58[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x0));   /* 0x5200[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           ( 0x106));   /* 0x6d06[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (   0x0));   /* 0x0000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5201[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6d06[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p709GHz_156p25MHz:    /* Divider value 68.5376   */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6000[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6000[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6800[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c00[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x44));   /* 0x6c44[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2268));   /* 0x2268[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x1));   /* 0x1040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c44[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p7545GHz_156p25MHz:   /* Divider value 68.8288   */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6d09[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6d09[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6d09[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6d09[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x44));   /* 0x6c44[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x350B));   /* 0x350b[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x1));   /* 0x1040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c44[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p9375GHz_156p25MHz:   /* Divider value 70        */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c44[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c44[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c44[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c44[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x0));   /* 0x5200[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0xE2));   /* 0x6ce2[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (   0x0));   /* 0x0000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5201[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6ce2[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p049GHz_156p25MHz:    /* Divider value 70.7136   */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6000[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6000[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6800[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c00[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x46));   /* 0x6c46[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2DAB));   /* 0x2dab[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xA));   /* 0xa040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c46[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p095GHz_156p25MHz:    /* Divider value 71.008    */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5204[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x7036[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6036[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6836[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c36[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x47));   /* 0x6c47[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (  0x83));   /* 0x0083[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x1));   /* 0x1040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c47[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p14273GHz_156p25MHz:  /* Divider value 71.31344  */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c44[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c44[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c44[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c44[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x47));   /* 0x6c47[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x140F));   /* 0x140f[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xF));   /* 0xf040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c47[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p181GHz_156p25MHz:    /* Divider value 71.5584   */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6000[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6000[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6800[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c00[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x47));   /* 0x6c47[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x23BC));   /* 0x23bc[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xD));   /* 0xd040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c47[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p25GHz_156p25MHz:     /* Divider value 72        */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6000[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6000[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6800[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c00[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x0));   /* 0x5200[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           ( 0x109));   /* 0x6d09[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (   0x0));   /* 0x0000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5201[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6d09[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p45863GHz_156p25MHz:  /* Divider value 73.335232 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c49[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c49[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c49[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c49[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x49));   /* 0x6c49[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x1574));   /* 0x1574[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x7));   /* 0x7040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c49[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p5GHz_156p25MHz:      /* Divider value 73.6      */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6ce2[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6ce2[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6ce2[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6ce2[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x49));   /* 0x6c49[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2666));   /* 0x2666[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x6));   /* 0x6040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c49[14:14] */
        break;

    case MERLIN_MPTWO_pll_8p5GHz_156p25MHz:       /* Divider value 54.4      */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
       EFUN(wrc_ams_pll_en_8p5g              (   0x1));
       EFUN(wrc_ams_pll_en_8p5g_vco          (   0x1));
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6000[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x1));   /* 0x7000[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x0));   /* 0x7000[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x0));   /* 0x7000[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x2));   /* 0x5204[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x36));   /* 0x7036[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x1999));   /* 0x1999[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xA));   /* 0xa040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5205[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x7036[14:14] */
        break;

    case MERLIN_MPTWO_pll_10GHz_161p132MHz:       /* Divider value 62.060919 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5204[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x7042[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x1));   /* 0x7042[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x0));   /* 0x7042[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x0));   /* 0x7042[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x2));   /* 0x5204[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x3E));   /* 0x703e[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        ( 0x3E1));   /* 0x03e6[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x2040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5205[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x703e[14:14] */
        break;


    case MERLIN_MPTWO_pll_10p709GHz_161p132MHz:   /* Divider value 66.461038 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5204[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x7040[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x1));   /* 0x7040[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x0));   /* 0x7040[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x0));   /* 0x7040[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x2));   /* 0x5204[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x42));   /* 0x7042[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x1D7C));   /* 0x1d81[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x3));   /* 0xa040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5205[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x7042[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p7545GHz_161p132MHz:  /* Divider value 66.743415 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c45[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x1));   /* 0x7c45[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x0));   /* 0x7445[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x0));   /* 0x7045[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x2));   /* 0x5204[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x42));   /* 0x7042[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2F8E));   /* 0x2f94[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xA));   /* 0x2040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5205[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x7042[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p9375GHz_161p132MHz:  /* Divider value 67.87913  */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5204[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x7042[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6042[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6842[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c42[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x43));   /* 0x6c43[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x383E));   /* 0x3843[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x1));   /* 0xb040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c43[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p049GHz_161p132MHz:   /* Divider value 68.571109 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c44[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c44[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c44[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c44[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x44));   /* 0x6c44[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x2487));   /* 0x248d[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x6));   /* 0x1040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c44[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p095GHz_161p132MHz:   /* Divider value 68.85659  */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c45[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c45[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c45[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c45[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x44));   /* 0x6c44[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x36CC));   /* 0x36d2[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xB));   /* 0x5040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c44[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p14273GHz_161p132MHz: /* Divider value 69.152806 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c47[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6c47[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6c47[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c47[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x45));   /* 0x6c45[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        ( 0x9C1));   /* 0x09c7[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xE));   /* 0x9040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c45[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p181GHz_161p132MHz:   /* Divider value 69.390314 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5204[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x703e[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x603e[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x683e[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c3e[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x45));   /* 0x6c45[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x18F5));   /* 0x18fa[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x3));   /* 0xe040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c45[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p25GHz_161p132MHz:    /* Divider value 69.818534 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5204[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x703e[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x603e[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x683e[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c3e[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x45));   /* 0x6c45[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x345D));   /* 0x3462[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x1));   /* 0xe040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c45[14:14] */
        break;

    case MERLIN_MPTWO_pll_11p45863GHz_161p132MHz: /* Divider value 71.113311 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5204[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x7040[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6040[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6840[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c40[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x47));   /* 0x6c47[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        ( 0x73A));   /* 0x0740[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xA));   /* 0x8040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6c47[14:14] */
        break;

    case MERLIN_MPTWO_pll_8p5GHz_161p132MHz:      /* Divider value 52.751781 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
       EFUN(wrc_ams_pll_en_8p5g              (   0x1));
       EFUN(wrc_ams_pll_en_8p5g_vco          (   0x1));
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c47[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x1));   /* 0x7c47[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x0));   /* 0x7447[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x0));   /* 0x7047[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x2));   /* 0x5204[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x34));   /* 0x7034[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x3018));   /* 0x301d[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xD));   /* 0x3040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5205[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x7034[14:14] */
        break;

    case MERLIN_MPTWO_pll_9p375GHz_161p132MHz:    /* Divider value 58.182112 */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5202[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6c44[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x1));   /* 0x7c44[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x0));   /* 0x7444[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x0));   /* 0x7044[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x2));   /* 0x5204[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0x3A));   /* 0x703a[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        ( 0xBA2));   /* 0x0ba7[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0xF));   /* 0xb040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5205[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x703a[14:14] */
        break;

    case MERLIN_MPTWO_pll_10GHz_50MHz:            /* Divider value 200       */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6000[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6000[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6800[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6c00[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x0));   /* 0x5200[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           ( 0x319));   /* 0x6f19[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (   0x0));   /* 0x0000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5201[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6f19[14:14] */
        break;

    case MERLIN_MPTWO_pll_10p3125GHz_50MHz:       /* Divider value 206.25    */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x0));   /* 0x5200[ 0: 0] */
        EFUN(wrc_ams_pll_div                  (   0x4));   /* 0x2106[15:11] */
        EFUN(wrc_ams_pll_mmd_en               (   0x1));   /* 0x6f19[13:13] */
        EFUN(wrc_ams_pll_mmd_prsc4or5pwdb     (   0x0));   /* 0x6f19[12:12] */
        EFUN(wrc_ams_pll_mmd_prsc8or9pwdb     (   0x1));   /* 0x6f19[11:11] */
        EFUN(wrc_ams_pll_mmd_div_range        (   0x1));   /* 0x6f19[10:10] */
        EFUN(wrc_ams_pll_i_pll_frac_mode      (   0x1));   /* 0x5202[ 2: 1] */
        EFUN(wrc_ams_pll_i_ndiv_int           (  0xCE));   /* 0x6cce[ 9: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_h        (0x1000));   /* 0x1000[13: 0] */
        EFUN(wrc_ams_pll_i_ndiv_frac_l        (   0x0));   /* 0x0040[15:12] */
        EFUN(wrc_ams_pll_mmd_resetb           (   0x1));   /* 0x5203[ 0: 0] */
        EFUN(wrc_ams_pll_i_pll_sdm_pwrdnb     (   0x1));   /* 0x6cce[14:14] */
        break;

    /*******************************/
    /*  Invalid 'pll_cfg' Selector */
    /*******************************/

    default:
        return _error(ERR_CODE_INVALID_PLL_CFG);
        break;

    }/* switch (pll_cfg) */

    /* Tail-chain to remainder to silence Coverity complaints due to prior
     * switch having eliminated all non-enuerated 'pll_cfg' values:  each
     * switch MUST "stand alone" with respect to parameter validation!
     */
    return merlin_mptwo_configure_pll_ctrl_bus(ma,pll_cfg);
}

static err_code_t merlin_mptwo_configure_pll_ctrl_bus(merlin_access_t *ma, enum merlin_mptwo_pll_enum pll_cfg)
{
    /* program PLL control bus settings */
    switch (pll_cfg) {                         /* Use Mode 1 settings below */
    case MERLIN_MPTWO_pll_8p5GHz_106p25MHz      : /* i_pll_ctrl[63:59] div<4:0> =  1100 */
    case MERLIN_MPTWO_pll_10GHz_125MHz          : /* i_pll_ctrl[63:59] div<4:0> =  1100 */
    case MERLIN_MPTWO_pll_10GHz_156p25MHz       : /* i_pll_ctrl[63:59] div<4:0> =   100 */
    case MERLIN_MPTWO_pll_10p3125GHz_156p25MHz  : /* i_pll_ctrl[63:59] div<4:0> = 10100 */
    case MERLIN_MPTWO_pll_9p375GHz_156p25MHz    : /* i_pll_ctrl[63:59] div<4:0> = 11100 */
    case MERLIN_MPTWO_pll_11p25GHz_125MHz       : /* Divider value 90        */
    case MERLIN_MPTWO_pll_8p5GHz_125MHz         : /* Divider value 68        */
    case MERLIN_MPTWO_pll_9p375GHz_125MHz       : /* Divider value 75        */
    case MERLIN_MPTWO_pll_10p9375GHz_156p25MHz  : /* Divider value 70        */
    case MERLIN_MPTWO_pll_11p25GHz_156p25MHz    : /* Divider value 72        */
        EFUN(_set_merlin_pll_mode1(ma));
        break;                                 /* Use Mode 2 settings below */
    case MERLIN_MPTWO_pll_10p3125GHz_125MHz     : /* Divider value 82.5      */
    case MERLIN_MPTWO_pll_10p709GHz_125MHz      : /* Divider value 85.672    */
    case MERLIN_MPTWO_pll_10p7545GHz_125MHz     : /* Divider value 86.036    */
    case MERLIN_MPTWO_pll_10p9375GHz_125MHz     : /* Divider value 87.5      */
    case MERLIN_MPTWO_pll_11p049GHz_125MHz      : /* Divider value 88.392    */
    case MERLIN_MPTWO_pll_11p095GHz_125MHz      : /* Divider value 88.76     */
    case MERLIN_MPTWO_pll_11p14273GHz_125MHz    : /* Divider value 89.14184  */
    case MERLIN_MPTWO_pll_11p181GHz_125MHz      : /* Divider value 89.448    */
    case MERLIN_MPTWO_pll_11p45863GHz_125MHz    : /* Divider value 91.66904  */
    case MERLIN_MPTWO_pll_10p709GHz_156p25MHz   : /* Divider value 68.5376   */
    case MERLIN_MPTWO_pll_10p7545GHz_156p25MHz  : /* Divider value 68.8288   */
    case MERLIN_MPTWO_pll_11p049GHz_156p25MHz   : /* Divider value 70.7136   */
    case MERLIN_MPTWO_pll_11p095GHz_156p25MHz   : /* Divider value 71.008    */
    case MERLIN_MPTWO_pll_11p14273GHz_156p25MHz : /* Divider value 71.313472 */
    case MERLIN_MPTWO_pll_11p181GHz_156p25MHz   : /* Divider value 71.5584   */
    case MERLIN_MPTWO_pll_11p45863GHz_156p25MHz : /* Divider value 73.335232 */
    case MERLIN_MPTWO_pll_11p5GHz_156p25MHz     : /* Divider value 73.6      */
    case MERLIN_MPTWO_pll_8p5GHz_156p25MHz      : /* Divider value 54.4      */
    case MERLIN_MPTWO_pll_10GHz_161p132MHz      : /* Divider value 62.060919 */
    case MERLIN_MPTWO_pll_10p3125GHz_161p132MHz : /* Divider value 64.000323 */
    case MERLIN_MPTWO_pll_10p709GHz_161p132MHz  : /* Divider value 66.461038 */
    case MERLIN_MPTWO_pll_10p7545GHz_161p132MHz : /* Divider value 66.743415 */
    case MERLIN_MPTWO_pll_10p9375GHz_161p132MHz : /* Divider value 67.87913  */
    case MERLIN_MPTWO_pll_11p049GHz_161p132MHz  : /* Divider value 68.571109 */
    case MERLIN_MPTWO_pll_11p095GHz_161p132MHz  : /* Divider value 68.85659  */
    case MERLIN_MPTWO_pll_11p14273GHz_161p132MHz: /* Divider value 69.152806 */
    case MERLIN_MPTWO_pll_11p181GHz_161p132MHz  : /* Divider value 69.390314 */
    case MERLIN_MPTWO_pll_11p25GHz_161p132MHz   : /* Divider value 69.818534 */
    case MERLIN_MPTWO_pll_11p45863GHz_161p132MHz: /* Divider value 71.113311 */
    case MERLIN_MPTWO_pll_8p5GHz_161p132MHz     : /* Divider value 52.751781 */
    case MERLIN_MPTWO_pll_9p375GHz_161p132MHz   : /* Divider value 58.182112 */
        EFUN(_set_merlin_pll_mode2(ma));
        break;                                 /* Use Mode 3 settings below */
    case MERLIN_MPTWO_pll_10GHz_50MHz           : /* i_pll_ctrl[63:59] div<4:0> =    10 */
    case MERLIN_MPTWO_pll_9p375GHz_50MHz        : /* i_pll_ctrl[63:59] div<4:0> = 11010 */
        EFUN(_set_merlin_pll_mode3(ma));
        break;                                 /* Use Mode 4 settings below */
    case MERLIN_MPTWO_pll_10p3125GHz_50MHz      : /* Divider value 206.25    */
        EFUN(_set_merlin_pll_mode4(ma));
        break;

    /*******************************/
    /*  Invalid 'pll_cfg' Selector */
    /*******************************/

    default:
       return (_error(ERR_CODE_INVALID_PLL_CFG));
       break;
    } /* switch (pll_cfg) */

    return (ERR_CODE_NONE);
} /* merlin_mptwo_configure_pll_ctrl_bus */

static err_code_t _set_merlin_pll_mode1(merlin_access_t *ma) {
    EFUN(wrc_ams_pll_curr_sel                ( 0x7));
    EFUN(wrc_ams_pll_rpar                    ( 0x4));
 /* EFUN(wrc_ams_pll_i_pfd_offset            (    )); *//* use default */
 /* EFUN(wrc_ams_pll_sel_fp3cap              (    )); *//* use default */
 /* EFUN(wrc_ams_pll_i_ndiv_dither_en        (    )); *//* use default */
 /* EFUN(wrc_ams_pll_cpar                    (    )); *//* use default */
    EFUN(wrc_ams_pll_refclk_in_bias          (0x3f));

    return (ERR_CODE_NONE);
}

static err_code_t _set_merlin_pll_mode2(merlin_access_t *ma) {
    EFUN(wrc_ams_pll_curr_sel                ( 0x3));
    EFUN(wrc_ams_pll_rpar                    ( 0x7));
    EFUN(wrc_ams_pll_pfd_offset              ( 0x2));
    EFUN(wrc_ams_pll_sel_fp3cap              ( 0xf));
    EFUN(wrc_ams_pll_i_ndiv_dither_en        ( 0x1));
 /* EFUN(wrc_ams_pll_cpar                    (    )); *//* use default */
    EFUN(wrc_ams_pll_refclk_in_bias          (0x3f));

    return (ERR_CODE_NONE);
}

static err_code_t _set_merlin_pll_mode3(merlin_access_t *ma) {
    EFUN(wrc_ams_pll_curr_sel                ( 0x7));
    EFUN(wrc_ams_pll_rpar                    ( 0x6));
 /* EFUN(wrc_ams_pll_i_pfd_offset            (    )); *//* use default */
 /* EFUN(wrc_ams_pll_sel_fp3cap              (    )); *//* use default */
 /* EFUN(wrc_ams_pll_i_ndiv_dither_en        (    )); *//* use default */
 /* EFUN(wrc_ams_pll_cpar                    (    )); *//* use default */
    EFUN(wrc_ams_pll_refclk_in_bias          (0x3f));

    return (ERR_CODE_NONE);
}

static err_code_t _set_merlin_pll_mode4(merlin_access_t *ma) {
    EFUN(wrc_ams_pll_curr_sel                ( 0x3));
    EFUN(wrc_ams_pll_rpar                    ( 0x7));
    EFUN(wrc_ams_pll_pfd_offset              ( 0x2));
    EFUN(wrc_ams_pll_sel_fp3cap              ( 0xf));
    EFUN(wrc_ams_pll_i_ndiv_dither_en        ( 0x1));
    EFUN(wrc_ams_pll_cpar                    ( 0x3));
    EFUN(wrc_ams_pll_refclk_in_bias          (0x3f));

    return (ERR_CODE_NONE);
}
