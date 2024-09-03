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


#ifndef _RDD_AG_DHD_TX_POST_H_
#define _RDD_AG_DHD_TX_POST_H_

int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_set(uint32_t _entry, uint16_t bias, uint16_t slope);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_set_core(uint32_t _entry, uint16_t bias, uint16_t slope, int core_id);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_get(uint32_t _entry, uint16_t *bias, uint16_t *slope);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_get_core(uint32_t _entry, uint16_t *bias, uint16_t *slope, int core_id);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_set(uint32_t _entry, uint16_t bias);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_set_core(uint32_t _entry, uint16_t bias, int core_id);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_get(uint32_t _entry, uint16_t *bias);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_get_core(uint32_t _entry, uint16_t *bias, int core_id);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_set(uint32_t _entry, uint16_t slope);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_set_core(uint32_t _entry, uint16_t slope, int core_id);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_get(uint32_t _entry, uint16_t *slope);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_get_core(uint32_t _entry, uint16_t *slope, int core_id);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_set(uint16_t fpm_thresholds_low);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_set_core(uint16_t fpm_thresholds_low, int core_id);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_get(uint16_t *fpm_thresholds_low);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_get_core(uint16_t *fpm_thresholds_low, int core_id);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_set(uint16_t fpm_thresholds_high);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_set_core(uint16_t fpm_thresholds_high, int core_id);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_get(uint16_t *fpm_thresholds_high);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_get_core(uint16_t *fpm_thresholds_high, int core_id);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_set(uint16_t fpm_thresholds_excl);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_set_core(uint16_t fpm_thresholds_excl, int core_id);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_get(uint16_t *fpm_thresholds_excl);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_get_core(uint16_t *fpm_thresholds_excl, int core_id);

#endif /* _RDD_AG_DHD_TX_POST_H_ */
