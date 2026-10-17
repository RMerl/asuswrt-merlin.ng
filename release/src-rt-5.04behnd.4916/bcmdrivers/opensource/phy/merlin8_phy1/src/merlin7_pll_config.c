/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

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


/**********************************************************************************
 **********************************************************************************
 *  File Name     :  merlin7_pll_config.c                                         *
 *  Created On    :  14/07/2013                                                   *
 *  Created By    :  Kiran Divakar                                                *
 *  Description   :  Merlin7 PLL Configuration API                                *
 *  Revision      :    *
 *                                                                                *
 **********************************************************************************
 **********************************************************************************/

/** @file merlin7_pll_config.c
 * Merlin7 PLL Configuration
 */

#ifndef PCIEG3_V1

#include "merlin8_phy1_config.h"
#include "merlin8_phy1_functions.h"
#include "merlin8_phy1_internal.h"
#include "merlin8_phy1_internal_error.h"
#include "merlin8_phy1_select_defns.h"

static err_code_t _merlin8_phy1_pll_powerdown(srds_access_t *sa__, uint8_t pwrdn_en);
static err_code_t _merlin8_phy1_restore_pll_defaults(srds_access_t *sa__);

#define pll_fraction_width 20

#define pll_fraction_width_h 14
#define pll_fraction_width_l (pll_fraction_width - pll_fraction_width_h)
#define pll_fraction_mask_h ((1<<pll_fraction_width_h)-1)
#define pll_fraction_mask_l ((1<<pll_fraction_width_l)-1)

#define _ndiv_frac_l(x) ((x)&pll_fraction_mask_l)
#define _ndiv_frac_h(x) ((x)>>pll_fraction_width_l)

#define _ndiv_frac_decode(l_, h_) (((l_) & pll_fraction_mask_l) | (((h_) & pll_fraction_mask_h) << pll_fraction_width_l))

static err_code_t _merlin8_phy1_pll_powerdown(srds_access_t *sa__, uint8_t pwrdn_en) {
    INIT_SRDS_ERR_CODE
    EFUN(wrc_ams_pll_afe_pll_pwrup_ena(1)); // Probably should be one time at config
    EFUN(wrc_ams_pll_pwrup_sel(1)); // Probably should be one time at config
    EFUN(wrc_ams_pll_pwrup(pwrdn_en?0:1));  // Invert boolean

    return (ERR_CODE_NONE);
}

static err_code_t _merlin8_phy1_restore_pll_defaults(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    // Assumes refclk = 100MHz
    // Values read manually from RDB.  These should be parsed by reg_field_rdwr_defns.pl instead.
    EFUN(wrc_ams_pll_ndiv_int(0x67));
    EFUN(wrc_ams_pll_ndiv_frac_h(0x800));
    EFUN(wrc_ams_pll_ndiv_frac_l(_ndiv_frac_l(0x0)));
    EFUN(wrc_ams_pll_refclk_div2_frc_val(0));
    EFUN(wrc_ams_pll_refclk_div4_frc_val(0));
    EFUN(wrc_ams_pll_refclk_div_frc(0));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_configure_pll(srds_access_t *sa__,
                                         enum merlin8_phy1_pll_refclk_enum refclk,
                                         enum merlin8_phy1_pll_div_enum srds_div,
                                         uint32_t vco_freq_khz,
                                         enum merlin8_phy1_pll_option_enum pll_option) {
    INIT_SRDS_ERR_CODE
    uint32_t refclk_freq_hz=0;

    if (pll_option & MERLIN8_PHY1_PLL_OPTION_POWERDOWN) {
        UNUSED(refclk_freq_hz);
        EFUN(_merlin8_phy1_pll_powerdown(sa__, 0x1));
        return (ERR_CODE_NONE);
    }

    EFUN(merlin8_phy1_INTERNAL_resolve_pll_parameters(sa__, refclk, &refclk_freq_hz, &srds_div, &vco_freq_khz, pll_option));
    /* Use this to restore defaults if reprogramming the PLL under dp-reset (typically Auto-Neg FW) */
    EFUN(_merlin8_phy1_restore_pll_defaults(sa__));

    {
        uint8_t reset_state;
        /* Use core_s_rstb to re-initialize all registers to default before calling this function. */
        ESTM(reset_state = rdc_core_dp_reset_state());

        if(reset_state < 7) {
            EFUN_PRINTF(("ERROR: merlin8_phy1_configure_pll(..) called without core_dp_s_rstb=0\n"));
            return (merlin8_phy1_error(sa__, ERR_CODE_CORE_DP_NOT_RESET));
        }
    }

    /* Clear PLL powerdown */
    EFUN(_merlin8_phy1_pll_powerdown(sa__, 0x0));

    /* Set PLL predivider and adjust srds_div as needed */
    EFUN(merlin8_phy1_INTERNAL_set_pll_pdiv(sa__,&srds_div, pll_fraction_width));

    EFUN(wrc_ams_pll_ndiv_int(SRDS_INTERNAL_GET_PLL_DIV_INTEGER(srds_div)));
    {
        const uint32_t pll_fraction_num = SRDS_INTERNAL_GET_PLL_DIV_FRACTION_NUM(srds_div, pll_fraction_width);
        EFUN(wrc_ams_pll_ndiv_frac_h   ((uint16_t)(_ndiv_frac_h(pll_fraction_num))));
        EFUN(wrc_ams_pll_ndiv_frac_l   (_ndiv_frac_l(pll_fraction_num)));

        pll_option = (enum merlin8_phy1_pll_option_enum)(pll_option & MERLIN8_PHY1_PLL_OPTION_REFCLK_MASK);

        /* Handle refclk PLL options */
/* WARNING: there seems to be an alternate set of registers with similar functionality */
        if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV2_EN) {
            EFUN(wrc_ams_pll_refclk_div2_frc_val(1));
            EFUN(wrc_ams_pll_refclk_div_frc(1));
        } else if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV4_EN) {
            EFUN(wrc_ams_pll_refclk_div4_frc_val(1));
            EFUN(wrc_ams_pll_refclk_div_frc(1));
        }
    }

    /* NOTE: Might have to add some optimized PLL control bus settings post-DVT (See 28nm merlin_pll_config.c) */

    /* Update core variables with the VCO rate. */
    {
        struct merlin8_phy1_uc_core_config_st core_config;
        EFUN(merlin8_phy1_get_uc_core_config(sa__, &core_config));
        core_config.vco_rate_in_Mhz = (int32_t)((vco_freq_khz + 500) / 1000);
        core_config.field.vco_rate = MHZ_TO_VCO_RATE(core_config.vco_rate_in_Mhz);
        EFUN(merlin8_phy1_INTERNAL_set_uc_core_config(sa__, core_config));
    }

    return (ERR_CODE_NONE);

} /* merlin8_phy1_configure_pll */

err_code_t merlin8_phy1_INTERNAL_read_pll_div(srds_access_t *sa__, uint32_t *srds_div) {
    INIT_SRDS_ERR_CODE
    uint16_t ndiv_int;
    uint32_t ndiv_frac;
    ESTM(ndiv_int = rdc_ams_pll_ndiv_int());
    ESTM(ndiv_frac = (uint32_t)(_ndiv_frac_decode(rdc_ams_pll_ndiv_frac_l(), rdc_ams_pll_ndiv_frac_h())));
    *srds_div = SRDS_INTERNAL_COMPOSE_PLL_DIV(ndiv_int, ndiv_frac, pll_fraction_width);
    return (ERR_CODE_NONE);
}

#endif /* not defined PCIEG3 */
