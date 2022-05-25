/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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


#ifndef _RDD_AG_DHD_TX_POST_H_
#define _RDD_AG_DHD_TX_POST_H_

int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_set(uint16_t fpm_thresholds_low);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_low_get(uint16_t *fpm_thresholds_low);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_set(uint16_t fpm_thresholds_high);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_high_get(uint16_t *fpm_thresholds_high);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_set(uint16_t fpm_thresholds_excl);
int rdd_ag_dhd_tx_post_dhd_hw_cfg_fpm_thresholds_excl_get(uint16_t *fpm_thresholds_excl);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_set(uint32_t _entry, uint16_t bias, uint16_t slope);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_get(uint32_t _entry, uint16_t *bias, uint16_t *slope);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_set(uint32_t _entry, uint16_t bias);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_bias_get(uint32_t _entry, uint16_t *bias);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_set(uint32_t _entry, uint16_t slope);
int rdd_ag_dhd_tx_post_dhd_codel_bias_slope_table_slope_get(uint32_t _entry, uint16_t *slope);

#endif /* _RDD_AG_DHD_TX_POST_H_ */
