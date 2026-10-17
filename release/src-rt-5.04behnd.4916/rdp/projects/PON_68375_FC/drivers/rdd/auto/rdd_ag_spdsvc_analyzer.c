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

#include "rdd_ag_spdsvc_analyzer.h"

int rdd_ag_spdsvc_analyzer_pd_fifo_table_set(uint32_t _entry, bdmf_boolean valid, bdmf_boolean headroom, bdmf_boolean dont_agg, bdmf_boolean mc_copy, bdmf_boolean reprocess, bdmf_boolean color, bdmf_boolean force_copy, uint16_t second_level_q_spdsvs, uint16_t first_level_q, bdmf_boolean flag_1588, bdmf_boolean coherent, uint8_t hn_crypto_ct_lkp_status, uint16_t serial_num, bdmf_boolean priority, bdmf_boolean ingress_cong, bdmf_boolean abs, uint8_t error_type_or_qm_fc_source, uint16_t packet_length, bdmf_boolean drop, bdmf_boolean target_mem_1, uint8_t cong_state_stream, bdmf_boolean is_emac, bdmf_boolean eh, uint16_t ingress_port, uint16_t union3, bdmf_boolean agg_pd, bdmf_boolean target_mem_0, uint32_t payload_offset_sop)
{
    if(_entry >= RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_SIZE || second_level_q_spdsvs >= 512 || first_level_q >= 512 || hn_crypto_ct_lkp_status >= 32 || serial_num >= 1024 || error_type_or_qm_fc_source >= 32 || packet_length >= 16384 || cong_state_stream >= 4 || ingress_port >= 4096 || union3 >= 16384 || payload_offset_sop >= 1073741824)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_WRITE_G(valid, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_WRITE_G(headroom, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_WRITE_G(dont_agg, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_WRITE_G(mc_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_WRITE_G(reprocess, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE_G(color, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE_G(force_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_WRITE_G(second_level_q_spdsvs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_G(first_level_q, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_WRITE_G(flag_1588, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_WRITE_G(coherent, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_WRITE_G(hn_crypto_ct_lkp_status, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE_G(serial_num, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE_G(priority, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_WRITE_G(ingress_cong, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_WRITE_G(abs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_WRITE_G(error_type_or_qm_fc_source, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_G(packet_length, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_WRITE_G(drop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_WRITE_G(target_mem_1, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_WRITE_G(cong_state_stream, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_WRITE_G(is_emac, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_WRITE_G(eh, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_WRITE_G(ingress_port, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_WRITE_G(union3, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_WRITE_G(agg_pd, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_WRITE_G(target_mem_0, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_WRITE_G(payload_offset_sop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_analyzer_pd_fifo_table_set_core(uint32_t _entry, bdmf_boolean valid, bdmf_boolean headroom, bdmf_boolean dont_agg, bdmf_boolean mc_copy, bdmf_boolean reprocess, bdmf_boolean color, bdmf_boolean force_copy, uint16_t second_level_q_spdsvs, uint16_t first_level_q, bdmf_boolean flag_1588, bdmf_boolean coherent, uint8_t hn_crypto_ct_lkp_status, uint16_t serial_num, bdmf_boolean priority, bdmf_boolean ingress_cong, bdmf_boolean abs, uint8_t error_type_or_qm_fc_source, uint16_t packet_length, bdmf_boolean drop, bdmf_boolean target_mem_1, uint8_t cong_state_stream, bdmf_boolean is_emac, bdmf_boolean eh, uint16_t ingress_port, uint16_t union3, bdmf_boolean agg_pd, bdmf_boolean target_mem_0, uint32_t payload_offset_sop, int core_id)
{
    if(_entry >= RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_SIZE || second_level_q_spdsvs >= 512 || first_level_q >= 512 || hn_crypto_ct_lkp_status >= 32 || serial_num >= 1024 || error_type_or_qm_fc_source >= 32 || packet_length >= 16384 || cong_state_stream >= 4 || ingress_port >= 4096 || union3 >= 16384 || payload_offset_sop >= 1073741824)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_WRITE_CORE(valid, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_WRITE_CORE(headroom, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_WRITE_CORE(dont_agg, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_WRITE_CORE(mc_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_WRITE_CORE(reprocess, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE_CORE(color, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE_CORE(force_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_WRITE_CORE(second_level_q_spdsvs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_CORE(first_level_q, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_WRITE_CORE(flag_1588, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_WRITE_CORE(coherent, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_WRITE_CORE(hn_crypto_ct_lkp_status, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE_CORE(serial_num, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE_CORE(priority, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_WRITE_CORE(ingress_cong, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_WRITE_CORE(abs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_WRITE_CORE(error_type_or_qm_fc_source, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_CORE(packet_length, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_WRITE_CORE(drop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_WRITE_CORE(target_mem_1, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_WRITE_CORE(cong_state_stream, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_WRITE_CORE(is_emac, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_WRITE_CORE(eh, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_WRITE_CORE(ingress_port, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_WRITE_CORE(union3, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_WRITE_CORE(agg_pd, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_WRITE_CORE(target_mem_0, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_WRITE_CORE(payload_offset_sop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_analyzer_pd_fifo_table_get(uint32_t _entry, bdmf_boolean *valid, bdmf_boolean *headroom, bdmf_boolean *dont_agg, bdmf_boolean *mc_copy, bdmf_boolean *reprocess, bdmf_boolean *color, bdmf_boolean *force_copy, uint16_t *second_level_q_spdsvs, uint16_t *first_level_q, bdmf_boolean *flag_1588, bdmf_boolean *coherent, uint8_t *hn_crypto_ct_lkp_status, uint16_t *serial_num, bdmf_boolean *priority, bdmf_boolean *ingress_cong, bdmf_boolean *abs, uint8_t *error_type_or_qm_fc_source, uint16_t *packet_length, bdmf_boolean *drop, bdmf_boolean *target_mem_1, uint8_t *cong_state_stream, bdmf_boolean *is_emac, bdmf_boolean *eh, uint16_t *ingress_port, uint16_t *union3, bdmf_boolean *agg_pd, bdmf_boolean *target_mem_0, uint32_t *payload_offset_sop)
{
    if(_entry >= RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_READ_G(*valid, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_READ_G(*headroom, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_READ_G(*dont_agg, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_READ_G(*mc_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_READ_G(*reprocess, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ_G(*color, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ_G(*force_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_READ_G(*second_level_q_spdsvs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_G(*first_level_q, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_READ_G(*flag_1588, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_READ_G(*coherent, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_READ_G(*hn_crypto_ct_lkp_status, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ_G(*serial_num, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ_G(*priority, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_READ_G(*ingress_cong, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_READ_G(*abs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_READ_G(*error_type_or_qm_fc_source, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ_G(*packet_length, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_READ_G(*drop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_READ_G(*target_mem_1, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_READ_G(*cong_state_stream, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_READ_G(*is_emac, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_READ_G(*eh, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_READ_G(*ingress_port, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_READ_G(*union3, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_READ_G(*agg_pd, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_READ_G(*target_mem_0, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_READ_G(*payload_offset_sop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_analyzer_pd_fifo_table_get_core(uint32_t _entry, bdmf_boolean *valid, bdmf_boolean *headroom, bdmf_boolean *dont_agg, bdmf_boolean *mc_copy, bdmf_boolean *reprocess, bdmf_boolean *color, bdmf_boolean *force_copy, uint16_t *second_level_q_spdsvs, uint16_t *first_level_q, bdmf_boolean *flag_1588, bdmf_boolean *coherent, uint8_t *hn_crypto_ct_lkp_status, uint16_t *serial_num, bdmf_boolean *priority, bdmf_boolean *ingress_cong, bdmf_boolean *abs, uint8_t *error_type_or_qm_fc_source, uint16_t *packet_length, bdmf_boolean *drop, bdmf_boolean *target_mem_1, uint8_t *cong_state_stream, bdmf_boolean *is_emac, bdmf_boolean *eh, uint16_t *ingress_port, uint16_t *union3, bdmf_boolean *agg_pd, bdmf_boolean *target_mem_0, uint32_t *payload_offset_sop, int core_id)
{
    if(_entry >= RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_READ_CORE(*valid, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_READ_CORE(*headroom, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_READ_CORE(*dont_agg, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_READ_CORE(*mc_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_READ_CORE(*reprocess, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ_CORE(*color, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ_CORE(*force_copy, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_READ_CORE(*second_level_q_spdsvs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_CORE(*first_level_q, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_READ_CORE(*flag_1588, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_READ_CORE(*coherent, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_READ_CORE(*hn_crypto_ct_lkp_status, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ_CORE(*serial_num, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ_CORE(*priority, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_READ_CORE(*ingress_cong, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_READ_CORE(*abs, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_READ_CORE(*error_type_or_qm_fc_source, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ_CORE(*packet_length, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_READ_CORE(*drop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_READ_CORE(*target_mem_1, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_READ_CORE(*cong_state_stream, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_READ_CORE(*is_emac, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_READ_CORE(*eh, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_READ_CORE(*ingress_port, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_READ_CORE(*union3, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_READ_CORE(*agg_pd, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_READ_CORE(*target_mem_0, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_READ_CORE(*payload_offset_sop, RDD_SPDSVC_ANALYZER_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

