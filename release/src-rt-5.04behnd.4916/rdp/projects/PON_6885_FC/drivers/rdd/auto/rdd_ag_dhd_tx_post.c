/*
   Copyright (c) 2025 Broadcom
   All Rights Reserved

    <:label-BRCM:2025:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"

#include "rdd_ag_dhd_tx_post.h"

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_set(uint16_t fpm_thresholds_low)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_LOW_WRITE_G(fpm_thresholds_low, RDD_DHD_HW_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_set_core(uint16_t fpm_thresholds_low, int core_id)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_LOW_WRITE_CORE(fpm_thresholds_low, RDD_DHD_HW_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_get(uint16_t *fpm_thresholds_low)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_LOW_READ_G(*fpm_thresholds_low, RDD_DHD_HW_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_get_core(uint16_t *fpm_thresholds_low, int core_id)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_LOW_READ_CORE(*fpm_thresholds_low, RDD_DHD_HW_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_set(uint16_t fpm_thresholds_high)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_HIGH_WRITE_G(fpm_thresholds_high, RDD_DHD_HW_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_set_core(uint16_t fpm_thresholds_high, int core_id)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_HIGH_WRITE_CORE(fpm_thresholds_high, RDD_DHD_HW_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_get(uint16_t *fpm_thresholds_high)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_HIGH_READ_G(*fpm_thresholds_high, RDD_DHD_HW_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_get_core(uint16_t *fpm_thresholds_high, int core_id)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_HIGH_READ_CORE(*fpm_thresholds_high, RDD_DHD_HW_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_set(uint16_t fpm_thresholds_excl)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_EXCL_WRITE_G(fpm_thresholds_excl, RDD_DHD_HW_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_set_core(uint16_t fpm_thresholds_excl, int core_id)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_EXCL_WRITE_CORE(fpm_thresholds_excl, RDD_DHD_HW_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_get(uint16_t *fpm_thresholds_excl)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_EXCL_READ_G(*fpm_thresholds_excl, RDD_DHD_HW_CFG_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_get_core(uint16_t *fpm_thresholds_excl, int core_id)
{
    RDD_DHD_HW_CONFIGURATION_FPM_THRESHOLDS_EXCL_READ_CORE(*fpm_thresholds_excl, RDD_DHD_HW_CFG_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

