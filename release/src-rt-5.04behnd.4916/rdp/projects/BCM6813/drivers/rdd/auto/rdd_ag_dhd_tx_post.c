/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"

#include "rdd_ag_dhd_tx_post.h"

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_set(uint32_t _entry, uint16_t bias, uint16_t slope)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_WRITE_G(bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);
    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_WRITE_G(slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_set_core(uint32_t _entry, uint16_t bias, uint16_t slope, int core_id)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_WRITE_CORE(bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_WRITE_CORE(slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_get(uint32_t _entry, uint16_t *bias, uint16_t *slope)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_READ_G(*bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);
    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_READ_G(*slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_get_core(uint32_t _entry, uint16_t *bias, uint16_t *slope, int core_id)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_READ_CORE(*bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_READ_CORE(*slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_set(uint32_t _entry, uint16_t bias)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_WRITE_G(bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_set_core(uint32_t _entry, uint16_t bias, int core_id)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_WRITE_CORE(bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_get(uint32_t _entry, uint16_t *bias)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_READ_G(*bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_get_core(uint32_t _entry, uint16_t *bias, int core_id)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_BIAS_READ_CORE(*bias, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_set(uint32_t _entry, uint16_t slope)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_WRITE_G(slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_set_core(uint32_t _entry, uint16_t slope, int core_id)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_WRITE_CORE(slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_get(uint32_t _entry, uint16_t *slope)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_READ_G(*slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_get_core(uint32_t _entry, uint16_t *slope, int core_id)
{
    if(_entry >= RDD_DHD_CODEL_BIAS_SLOPE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_DHD_CODEL_BIAS_SLOPE_SLOPE_READ_CORE(*slope, RDD_DHD_CODEL_BIAS_SLOPE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

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

