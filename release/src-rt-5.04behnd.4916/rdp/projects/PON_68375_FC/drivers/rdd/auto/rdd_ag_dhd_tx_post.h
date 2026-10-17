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


#ifndef _RDD_AG_DHD_TX_POST_H_
#define _RDD_AG_DHD_TX_POST_H_

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
int rdd_ag_dhd_tx_post_pd_fifo_table_set(uint32_t _entry, bdmf_boolean valid, bdmf_boolean headroom, bdmf_boolean dont_agg, bdmf_boolean mc_copy, bdmf_boolean reprocess, bdmf_boolean color, bdmf_boolean force_copy, uint16_t second_level_q_spdsvs, uint16_t first_level_q, bdmf_boolean flag_1588, bdmf_boolean coherent, uint8_t hn_crypto_ct_lkp_status, uint16_t serial_num, bdmf_boolean priority, bdmf_boolean ingress_cong, bdmf_boolean abs, uint8_t error_type_or_qm_fc_source, uint16_t packet_length, bdmf_boolean drop, bdmf_boolean target_mem_1, uint8_t cong_state_stream, bdmf_boolean is_emac, bdmf_boolean eh, uint16_t ingress_port, uint16_t union3, bdmf_boolean agg_pd, bdmf_boolean target_mem_0, uint32_t payload_offset_sop);
int rdd_ag_dhd_tx_post_pd_fifo_table_set_core(uint32_t _entry, bdmf_boolean valid, bdmf_boolean headroom, bdmf_boolean dont_agg, bdmf_boolean mc_copy, bdmf_boolean reprocess, bdmf_boolean color, bdmf_boolean force_copy, uint16_t second_level_q_spdsvs, uint16_t first_level_q, bdmf_boolean flag_1588, bdmf_boolean coherent, uint8_t hn_crypto_ct_lkp_status, uint16_t serial_num, bdmf_boolean priority, bdmf_boolean ingress_cong, bdmf_boolean abs, uint8_t error_type_or_qm_fc_source, uint16_t packet_length, bdmf_boolean drop, bdmf_boolean target_mem_1, uint8_t cong_state_stream, bdmf_boolean is_emac, bdmf_boolean eh, uint16_t ingress_port, uint16_t union3, bdmf_boolean agg_pd, bdmf_boolean target_mem_0, uint32_t payload_offset_sop, int core_id);
int rdd_ag_dhd_tx_post_pd_fifo_table_get(uint32_t _entry, bdmf_boolean *valid, bdmf_boolean *headroom, bdmf_boolean *dont_agg, bdmf_boolean *mc_copy, bdmf_boolean *reprocess, bdmf_boolean *color, bdmf_boolean *force_copy, uint16_t *second_level_q_spdsvs, uint16_t *first_level_q, bdmf_boolean *flag_1588, bdmf_boolean *coherent, uint8_t *hn_crypto_ct_lkp_status, uint16_t *serial_num, bdmf_boolean *priority, bdmf_boolean *ingress_cong, bdmf_boolean *abs, uint8_t *error_type_or_qm_fc_source, uint16_t *packet_length, bdmf_boolean *drop, bdmf_boolean *target_mem_1, uint8_t *cong_state_stream, bdmf_boolean *is_emac, bdmf_boolean *eh, uint16_t *ingress_port, uint16_t *union3, bdmf_boolean *agg_pd, bdmf_boolean *target_mem_0, uint32_t *payload_offset_sop);
int rdd_ag_dhd_tx_post_pd_fifo_table_get_core(uint32_t _entry, bdmf_boolean *valid, bdmf_boolean *headroom, bdmf_boolean *dont_agg, bdmf_boolean *mc_copy, bdmf_boolean *reprocess, bdmf_boolean *color, bdmf_boolean *force_copy, uint16_t *second_level_q_spdsvs, uint16_t *first_level_q, bdmf_boolean *flag_1588, bdmf_boolean *coherent, uint8_t *hn_crypto_ct_lkp_status, uint16_t *serial_num, bdmf_boolean *priority, bdmf_boolean *ingress_cong, bdmf_boolean *abs, uint8_t *error_type_or_qm_fc_source, uint16_t *packet_length, bdmf_boolean *drop, bdmf_boolean *target_mem_1, uint8_t *cong_state_stream, bdmf_boolean *is_emac, bdmf_boolean *eh, uint16_t *ingress_port, uint16_t *union3, bdmf_boolean *agg_pd, bdmf_boolean *target_mem_0, uint32_t *payload_offset_sop, int core_id);

#endif /* _RDD_AG_DHD_TX_POST_H_ */
