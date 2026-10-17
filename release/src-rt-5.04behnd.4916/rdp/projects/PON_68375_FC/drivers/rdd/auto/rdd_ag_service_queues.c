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

#include "rdd_ag_service_queues.h"

int rdd_ag_service_queues_dcl_data_base_set(uint32_t rnr_address, uint16_t size, bdmf_boolean dcl_act, uint32_t ddr_addr_h, uint32_t ddr_addr_l, uint32_t stack_addr, uint32_t stack_addr_1, uint32_t stack_addr_2, uint32_t task_addr)
{
    if(size >= 32768)
          return BDMF_ERR_PARM;

    RDD_DCL_DATA_RNR_ADDRESS_WRITE_G(rnr_address, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_SIZE_WRITE_G(size, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_DCL_ACT_WRITE_G(dcl_act, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_DDR_ADDR_H_WRITE_G(ddr_addr_h, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_DDR_ADDR_L_WRITE_G(ddr_addr_l, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_STACK_ADDR_WRITE_G(stack_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_STACK_ADDR_1_WRITE_G(stack_addr_1, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_STACK_ADDR_2_WRITE_G(stack_addr_2, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_TASK_ADDR_WRITE_G(task_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_dcl_data_base_set_core(uint32_t rnr_address, uint16_t size, bdmf_boolean dcl_act, uint32_t ddr_addr_h, uint32_t ddr_addr_l, uint32_t stack_addr, uint32_t stack_addr_1, uint32_t stack_addr_2, uint32_t task_addr, int core_id)
{
    if(size >= 32768)
          return BDMF_ERR_PARM;

    RDD_DCL_DATA_RNR_ADDRESS_WRITE_CORE(rnr_address, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_SIZE_WRITE_CORE(size, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_DCL_ACT_WRITE_CORE(dcl_act, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_DDR_ADDR_H_WRITE_CORE(ddr_addr_h, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_DDR_ADDR_L_WRITE_CORE(ddr_addr_l, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_STACK_ADDR_WRITE_CORE(stack_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_STACK_ADDR_1_WRITE_CORE(stack_addr_1, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_STACK_ADDR_2_WRITE_CORE(stack_addr_2, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_TASK_ADDR_WRITE_CORE(task_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_dcl_data_base_get(uint32_t *rnr_address, uint16_t *size, bdmf_boolean *dcl_act, uint32_t *ddr_addr_h, uint32_t *ddr_addr_l, uint32_t *stack_addr, uint32_t *stack_addr_1, uint32_t *stack_addr_2, uint32_t *task_addr)
{
    RDD_DCL_DATA_RNR_ADDRESS_READ_G(*rnr_address, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_SIZE_READ_G(*size, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_DCL_ACT_READ_G(*dcl_act, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_DDR_ADDR_H_READ_G(*ddr_addr_h, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_DDR_ADDR_L_READ_G(*ddr_addr_l, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_STACK_ADDR_READ_G(*stack_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_STACK_ADDR_1_READ_G(*stack_addr_1, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_STACK_ADDR_2_READ_G(*stack_addr_2, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);
    RDD_DCL_DATA_TASK_ADDR_READ_G(*task_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_dcl_data_base_get_core(uint32_t *rnr_address, uint16_t *size, bdmf_boolean *dcl_act, uint32_t *ddr_addr_h, uint32_t *ddr_addr_l, uint32_t *stack_addr, uint32_t *stack_addr_1, uint32_t *stack_addr_2, uint32_t *task_addr, int core_id)
{
    RDD_DCL_DATA_RNR_ADDRESS_READ_CORE(*rnr_address, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_SIZE_READ_CORE(*size, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_DCL_ACT_READ_CORE(*dcl_act, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_DDR_ADDR_H_READ_CORE(*ddr_addr_h, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_DDR_ADDR_L_READ_CORE(*ddr_addr_l, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_STACK_ADDR_READ_CORE(*stack_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_STACK_ADDR_1_READ_CORE(*stack_addr_1, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_STACK_ADDR_2_READ_CORE(*stack_addr_2, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);
    RDD_DCL_DATA_TASK_ADDR_READ_CORE(*task_addr, RDD_SQ_TM_DCL_DATA_BASE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_dcl_img_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_SQ_TM_DCL_IMG_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_SQ_TM_DCL_IMG_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_dcl_img_set_core(uint32_t _entry, uint8_t bits, int core_id)
{
    if(_entry >= RDD_SQ_TM_DCL_IMG_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_CORE(bits, RDD_SQ_TM_DCL_IMG_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_dcl_img_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_SQ_TM_DCL_IMG_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_SQ_TM_DCL_IMG_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_dcl_img_get_core(uint32_t _entry, uint8_t *bits, int core_id)
{
    if(_entry >= RDD_SQ_TM_DCL_IMG_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_CORE(*bits, RDD_SQ_TM_DCL_IMG_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_first_queue_mapping_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_SQ_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_first_queue_mapping_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_SQ_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_first_queue_mapping_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_SQ_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_first_queue_mapping_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_SQ_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_pd_fifo_table_set(uint32_t _entry, bdmf_boolean valid, bdmf_boolean headroom, bdmf_boolean dont_agg, bdmf_boolean mc_copy, bdmf_boolean reprocess, bdmf_boolean color, bdmf_boolean force_copy, uint16_t second_level_q_spdsvs, uint16_t first_level_q, bdmf_boolean flag_1588, bdmf_boolean coherent, uint8_t hn_crypto_ct_lkp_status, uint16_t serial_num, bdmf_boolean priority, bdmf_boolean ingress_cong, bdmf_boolean abs, uint8_t error_type_or_qm_fc_source, uint16_t packet_length, bdmf_boolean drop, bdmf_boolean target_mem_1, uint8_t cong_state_stream, bdmf_boolean is_emac, bdmf_boolean eh, uint16_t ingress_port, uint16_t union3, bdmf_boolean agg_pd, bdmf_boolean target_mem_0, uint32_t payload_offset_sop)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE || second_level_q_spdsvs >= 512 || first_level_q >= 512 || hn_crypto_ct_lkp_status >= 32 || serial_num >= 1024 || error_type_or_qm_fc_source >= 32 || packet_length >= 16384 || cong_state_stream >= 4 || ingress_port >= 4096 || union3 >= 16384 || payload_offset_sop >= 1073741824)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_WRITE_G(valid, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_WRITE_G(headroom, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_WRITE_G(dont_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_WRITE_G(mc_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_WRITE_G(reprocess, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE_G(color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE_G(force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_WRITE_G(second_level_q_spdsvs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_G(first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_WRITE_G(flag_1588, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_WRITE_G(coherent, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_WRITE_G(hn_crypto_ct_lkp_status, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE_G(serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE_G(priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_WRITE_G(ingress_cong, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_WRITE_G(abs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_WRITE_G(error_type_or_qm_fc_source, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_G(packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_WRITE_G(drop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_WRITE_G(target_mem_1, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_WRITE_G(cong_state_stream, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_WRITE_G(is_emac, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_WRITE_G(eh, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_WRITE_G(ingress_port, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_WRITE_G(union3, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_WRITE_G(agg_pd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_WRITE_G(target_mem_0, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_WRITE_G(payload_offset_sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_pd_fifo_table_set_core(uint32_t _entry, bdmf_boolean valid, bdmf_boolean headroom, bdmf_boolean dont_agg, bdmf_boolean mc_copy, bdmf_boolean reprocess, bdmf_boolean color, bdmf_boolean force_copy, uint16_t second_level_q_spdsvs, uint16_t first_level_q, bdmf_boolean flag_1588, bdmf_boolean coherent, uint8_t hn_crypto_ct_lkp_status, uint16_t serial_num, bdmf_boolean priority, bdmf_boolean ingress_cong, bdmf_boolean abs, uint8_t error_type_or_qm_fc_source, uint16_t packet_length, bdmf_boolean drop, bdmf_boolean target_mem_1, uint8_t cong_state_stream, bdmf_boolean is_emac, bdmf_boolean eh, uint16_t ingress_port, uint16_t union3, bdmf_boolean agg_pd, bdmf_boolean target_mem_0, uint32_t payload_offset_sop, int core_id)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE || second_level_q_spdsvs >= 512 || first_level_q >= 512 || hn_crypto_ct_lkp_status >= 32 || serial_num >= 1024 || error_type_or_qm_fc_source >= 32 || packet_length >= 16384 || cong_state_stream >= 4 || ingress_port >= 4096 || union3 >= 16384 || payload_offset_sop >= 1073741824)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_WRITE_CORE(valid, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_WRITE_CORE(headroom, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_WRITE_CORE(dont_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_WRITE_CORE(mc_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_WRITE_CORE(reprocess, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE_CORE(color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE_CORE(force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_WRITE_CORE(second_level_q_spdsvs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_CORE(first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_WRITE_CORE(flag_1588, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_WRITE_CORE(coherent, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_WRITE_CORE(hn_crypto_ct_lkp_status, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE_CORE(serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE_CORE(priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_WRITE_CORE(ingress_cong, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_WRITE_CORE(abs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_WRITE_CORE(error_type_or_qm_fc_source, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_CORE(packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_WRITE_CORE(drop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_WRITE_CORE(target_mem_1, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_WRITE_CORE(cong_state_stream, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_WRITE_CORE(is_emac, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_WRITE_CORE(eh, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_WRITE_CORE(ingress_port, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_WRITE_CORE(union3, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_WRITE_CORE(agg_pd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_WRITE_CORE(target_mem_0, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_WRITE_CORE(payload_offset_sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_pd_fifo_table_get(uint32_t _entry, bdmf_boolean *valid, bdmf_boolean *headroom, bdmf_boolean *dont_agg, bdmf_boolean *mc_copy, bdmf_boolean *reprocess, bdmf_boolean *color, bdmf_boolean *force_copy, uint16_t *second_level_q_spdsvs, uint16_t *first_level_q, bdmf_boolean *flag_1588, bdmf_boolean *coherent, uint8_t *hn_crypto_ct_lkp_status, uint16_t *serial_num, bdmf_boolean *priority, bdmf_boolean *ingress_cong, bdmf_boolean *abs, uint8_t *error_type_or_qm_fc_source, uint16_t *packet_length, bdmf_boolean *drop, bdmf_boolean *target_mem_1, uint8_t *cong_state_stream, bdmf_boolean *is_emac, bdmf_boolean *eh, uint16_t *ingress_port, uint16_t *union3, bdmf_boolean *agg_pd, bdmf_boolean *target_mem_0, uint32_t *payload_offset_sop)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_READ_G(*valid, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_READ_G(*headroom, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_READ_G(*dont_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_READ_G(*mc_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_READ_G(*reprocess, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ_G(*color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ_G(*force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_READ_G(*second_level_q_spdsvs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_G(*first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_READ_G(*flag_1588, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_READ_G(*coherent, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_READ_G(*hn_crypto_ct_lkp_status, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ_G(*serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ_G(*priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_READ_G(*ingress_cong, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_READ_G(*abs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_READ_G(*error_type_or_qm_fc_source, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ_G(*packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_READ_G(*drop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_READ_G(*target_mem_1, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_READ_G(*cong_state_stream, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_READ_G(*is_emac, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_READ_G(*eh, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_READ_G(*ingress_port, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_READ_G(*union3, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_READ_G(*agg_pd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_READ_G(*target_mem_0, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_READ_G(*payload_offset_sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_pd_fifo_table_get_core(uint32_t _entry, bdmf_boolean *valid, bdmf_boolean *headroom, bdmf_boolean *dont_agg, bdmf_boolean *mc_copy, bdmf_boolean *reprocess, bdmf_boolean *color, bdmf_boolean *force_copy, uint16_t *second_level_q_spdsvs, uint16_t *first_level_q, bdmf_boolean *flag_1588, bdmf_boolean *coherent, uint8_t *hn_crypto_ct_lkp_status, uint16_t *serial_num, bdmf_boolean *priority, bdmf_boolean *ingress_cong, bdmf_boolean *abs, uint8_t *error_type_or_qm_fc_source, uint16_t *packet_length, bdmf_boolean *drop, bdmf_boolean *target_mem_1, uint8_t *cong_state_stream, bdmf_boolean *is_emac, bdmf_boolean *eh, uint16_t *ingress_port, uint16_t *union3, bdmf_boolean *agg_pd, bdmf_boolean *target_mem_0, uint32_t *payload_offset_sop, int core_id)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_VALID_READ_CORE(*valid, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_READ_CORE(*headroom, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_READ_CORE(*dont_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_READ_CORE(*mc_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_READ_CORE(*reprocess, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ_CORE(*color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ_CORE(*force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_SPDSVS_READ_CORE(*second_level_q_spdsvs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_CORE(*first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_READ_CORE(*flag_1588, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_READ_CORE(*coherent, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HN_CRYPTO_CT_LKP_STATUS_READ_CORE(*hn_crypto_ct_lkp_status, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ_CORE(*serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ_CORE(*priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_READ_CORE(*ingress_cong, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ABS_READ_CORE(*abs, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ERROR_TYPE_OR_QM_FC_SOURCE_READ_CORE(*error_type_or_qm_fc_source, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ_CORE(*packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DROP_READ_CORE(*drop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_READ_CORE(*target_mem_1, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_STREAM_READ_CORE(*cong_state_stream, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_IS_EMAC_READ_CORE(*is_emac, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_EH_READ_CORE(*eh, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_READ_CORE(*ingress_port, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_UNION3_READ_CORE(*union3, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_READ_CORE(*agg_pd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_READ_CORE(*target_mem_0, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_SOP_READ_CORE(*payload_offset_sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_set(bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector)
{
    if(sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || bbh_queue_desc_id >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(is_positive_budget, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(rate_limit_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(rate_limiter_index, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_G(last_served_block, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_WRITE_G(bbh_queue_desc_id, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_WRITE_G(aqm_stats_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_G(status_bit_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_G(slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE_G(slot_budget_bit_vector_1, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_WRITE_G(secondary_scheduler_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_set_core(bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint8_t bbh_queue_desc_id, bdmf_boolean aqm_stats_enable, uint32_t status_bit_vector, uint32_t slot_budget_bit_vector_0, uint32_t slot_budget_bit_vector_1, uint32_t secondary_scheduler_vector, int core_id)
{
    if(sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || bbh_queue_desc_id >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_CORE(is_positive_budget, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_CORE(rate_limit_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_CORE(rate_limiter_index, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_CORE(last_served_block, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_WRITE_CORE(bbh_queue_desc_id, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_WRITE_CORE(aqm_stats_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_CORE(status_bit_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_CORE(slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE_CORE(slot_budget_bit_vector_1, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_WRITE_CORE(secondary_scheduler_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_get(bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector)
{
    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(*is_positive_budget, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(*rate_limit_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(*rate_limiter_index, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_G(*last_served_block, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_READ_G(*bbh_queue_desc_id, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_READ_G(*aqm_stats_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_G(*status_bit_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_G(*slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ_G(*slot_budget_bit_vector_1, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_READ_G(*secondary_scheduler_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_get_core(bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint8_t *bbh_queue_desc_id, bdmf_boolean *aqm_stats_enable, uint32_t *status_bit_vector, uint32_t *slot_budget_bit_vector_0, uint32_t *slot_budget_bit_vector_1, uint32_t *secondary_scheduler_vector, int core_id)
{
    RDD_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_CORE(*is_positive_budget, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_CORE(*rate_limit_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_CORE(*rate_limiter_index, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_CORE(*last_served_block, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_BBH_QUEUE_DESC_ID_READ_CORE(*bbh_queue_desc_id, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_AQM_STATS_ENABLE_READ_CORE(*aqm_stats_enable, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_CORE(*status_bit_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_CORE(*slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ_CORE(*slot_budget_bit_vector_1, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SCHEDULER_DESCRIPTOR_SECONDARY_SCHEDULER_VECTOR_READ_CORE(*secondary_scheduler_vector, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_sir_dwrr_offset_set(uint8_t sir_dwrr_offset)
{
    if(sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_sir_dwrr_offset_set_core(uint8_t sir_dwrr_offset, int core_id)
{
    if(sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_sir_dwrr_offset_get(uint8_t *sir_dwrr_offset)
{
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_sir_dwrr_offset_get_core(uint8_t *sir_dwrr_offset, int core_id)
{
    RDD_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_pir_dwrr_offset_set(uint8_t pir_dwrr_offset)
{
    if(pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_pir_dwrr_offset_set_core(uint8_t pir_dwrr_offset, int core_id)
{
    if(pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_pir_dwrr_offset_get(uint8_t *pir_dwrr_offset)
{
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_pir_dwrr_offset_get_core(uint8_t *pir_dwrr_offset, int core_id)
{
    RDD_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_queue_offset_set(uint16_t queue_offset)
{
    if(queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_queue_offset_set_core(uint16_t queue_offset, int core_id)
{
    if(queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_queue_offset_get(uint16_t *queue_offset)
{
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_queue_offset_get_core(uint16_t *queue_offset, int core_id)
{
    RDD_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_slot_budget_bit_vector_0_set(uint32_t slot_budget_bit_vector_0)
{
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_G(slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_slot_budget_bit_vector_0_set_core(uint32_t slot_budget_bit_vector_0, int core_id)
{
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_CORE(slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_slot_budget_bit_vector_0_get(uint32_t *slot_budget_bit_vector_0)
{
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_G(*slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduler_table_slot_budget_bit_vector_0_get_core(uint32_t *slot_budget_bit_vector_0, int core_id)
{
    RDD_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_CORE(*slot_budget_bit_vector_0, RDD_SQ_TM_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_bbh_queue_index_set(uint32_t _entry, uint8_t bbh_queue_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE || bbh_queue_index >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_WRITE_G(bbh_queue_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_bbh_queue_index_set_core(uint32_t _entry, uint8_t bbh_queue_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE || bbh_queue_index >= 64)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_WRITE_CORE(bbh_queue_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_bbh_queue_index_get(uint32_t _entry, uint8_t *bbh_queue_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_READ_G(*bbh_queue_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_bbh_queue_index_get_core(uint32_t _entry, uint8_t *bbh_queue_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_READ_CORE(*bbh_queue_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_enable_set(uint32_t _entry, bdmf_boolean enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_WRITE_G(enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_enable_set_core(uint32_t _entry, bdmf_boolean enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_WRITE_CORE(enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_enable_get(uint32_t _entry, bdmf_boolean *enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_READ_G(*enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_enable_get_core(uint32_t _entry, bdmf_boolean *enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_READ_CORE(*enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_set(uint32_t _entry, bdmf_boolean rate_limit_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean rate_limit_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_CORE(rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *rate_limit_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(*rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *rate_limit_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_CORE(*rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_qstat_enable_set(uint32_t _entry, bdmf_boolean qstat_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QSTAT_ENABLE_WRITE_G(qstat_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_qstat_enable_set_core(uint32_t _entry, bdmf_boolean qstat_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QSTAT_ENABLE_WRITE_CORE(qstat_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_qstat_enable_get(uint32_t _entry, bdmf_boolean *qstat_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QSTAT_ENABLE_READ_G(*qstat_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_qstat_enable_get_core(uint32_t _entry, bdmf_boolean *qstat_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QSTAT_ENABLE_READ_CORE(*qstat_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pi2_enable_set(uint32_t _entry, bdmf_boolean pi2_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_WRITE_G(pi2_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pi2_enable_set_core(uint32_t _entry, bdmf_boolean pi2_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_WRITE_CORE(pi2_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pi2_enable_get(uint32_t _entry, bdmf_boolean *pi2_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_READ_G(*pi2_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pi2_enable_get_core(uint32_t _entry, bdmf_boolean *pi2_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PI2_ENABLE_READ_CORE(*pi2_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_aqm_enable_set(uint32_t _entry, bdmf_boolean aqm_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_WRITE_G(aqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_aqm_enable_set_core(uint32_t _entry, bdmf_boolean aqm_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_WRITE_CORE(aqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_aqm_enable_get(uint32_t _entry, bdmf_boolean *aqm_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_READ_G(*aqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_aqm_enable_get_core(uint32_t _entry, bdmf_boolean *aqm_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_AQM_ENABLE_READ_CORE(*aqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_laqm_enable_set(uint32_t _entry, bdmf_boolean laqm_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_WRITE_G(laqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_laqm_enable_set_core(uint32_t _entry, bdmf_boolean laqm_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_WRITE_CORE(laqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_laqm_enable_get(uint32_t _entry, bdmf_boolean *laqm_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_READ_G(*laqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_laqm_enable_get_core(uint32_t _entry, bdmf_boolean *laqm_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_LAQM_ENABLE_READ_CORE(*laqm_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean sir_rate_limit_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_WRITE_G(sir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean sir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_WRITE_CORE(sir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_READ_G(*sir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *sir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMIT_ENABLE_READ_CORE(*sir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limit_enable_set(uint32_t _entry, bdmf_boolean pir_rate_limit_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_G(pir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limit_enable_set_core(uint32_t _entry, bdmf_boolean pir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_CORE(pir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limit_enable_get(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_G(*pir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limit_enable_get_core(uint32_t _entry, bdmf_boolean *pir_rate_limit_enable, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_CORE(*pir_rate_limit_enable, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_scheduler_index_set(uint32_t _entry, uint8_t scheduler_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE || scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_G(scheduler_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_scheduler_index_set_core(uint32_t _entry, uint8_t scheduler_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE || scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_CORE(scheduler_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_scheduler_index_get(uint32_t _entry, uint8_t *scheduler_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_G(*scheduler_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_scheduler_index_get_core(uint32_t _entry, uint8_t *scheduler_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_CORE(*scheduler_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limiter_index_set(uint32_t _entry, uint8_t sir_rate_limiter_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_WRITE_G(sir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limiter_index_set_core(uint32_t _entry, uint8_t sir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_WRITE_CORE(sir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limiter_index_get(uint32_t _entry, uint8_t *sir_rate_limiter_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_READ_G(*sir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_sir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *sir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SIR_RATE_LIMITER_INDEX_READ_CORE(*sir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_quantum_number_set_core(uint32_t _entry, uint8_t quantum_number, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_WRITE_CORE(quantum_number, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_quantum_number_get_core(uint32_t _entry, uint8_t *quantum_number, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ_CORE(*quantum_number, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limiter_index_set(uint32_t _entry, uint8_t pir_rate_limiter_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_G(pir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limiter_index_set_core(uint32_t _entry, uint8_t pir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_CORE(pir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limiter_index_get(uint32_t _entry, uint8_t *pir_rate_limiter_index)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_G(*pir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_scheduling_queue_table_pir_rate_limiter_index_get_core(uint32_t _entry, uint8_t *pir_rate_limiter_index, int core_id)
{
    if(_entry >= RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_SCHEDULING_QUEUE_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_CORE(*pir_rate_limiter_index, RDD_SQ_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_set(bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index)
{
    if(sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || status_bit_vector >= 16 || primary_scheduler_slot_index >= 32 || primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(is_positive_budget, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_G(last_served_block, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE_G(deficit_counter, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_G(pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_G(status_bit_vector, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_G(primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_G(pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_G(primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_set_core(bdmf_boolean is_positive_budget, uint8_t sir_dwrr_offset, uint8_t pir_dwrr_offset, bdmf_boolean rate_limit_enable, uint8_t rate_limiter_index, uint8_t last_served_block, uint16_t queue_offset, uint16_t deficit_counter, uint8_t quantum_number, uint8_t pir_rate_limiter_index, uint8_t status_bit_vector, uint8_t primary_scheduler_slot_index, bdmf_boolean pir_rate_limit_enable, uint8_t primary_scheduler_index, int core_id)
{
    if(sir_dwrr_offset >= 8 || pir_dwrr_offset >= 8 || rate_limiter_index >= 128 || last_served_block >= 128 || queue_offset >= 512 || status_bit_vector >= 16 || primary_scheduler_slot_index >= 32 || primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_CORE(is_positive_budget, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_CORE(rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_CORE(rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_WRITE_CORE(last_served_block, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE_CORE(deficit_counter, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_CORE(quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_CORE(pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_CORE(status_bit_vector, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_CORE(primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_CORE(pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_CORE(primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_get(bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(*is_positive_budget, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(*rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(*rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_G(*last_served_block, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ_G(*deficit_counter, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_G(*pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_G(*status_bit_vector, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_G(*primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_G(*pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_G(*primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_get_core(bdmf_boolean *is_positive_budget, uint8_t *sir_dwrr_offset, uint8_t *pir_dwrr_offset, bdmf_boolean *rate_limit_enable, uint8_t *rate_limiter_index, uint8_t *last_served_block, uint16_t *queue_offset, uint16_t *deficit_counter, uint8_t *quantum_number, uint8_t *pir_rate_limiter_index, uint8_t *status_bit_vector, uint8_t *primary_scheduler_slot_index, bdmf_boolean *pir_rate_limit_enable, uint8_t *primary_scheduler_index, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_CORE(*is_positive_budget, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_CORE(*rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_CORE(*rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_READ_CORE(*last_served_block, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ_CORE(*deficit_counter, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_CORE(*quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_CORE(*pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_CORE(*status_bit_vector, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_CORE(*primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_CORE(*pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_CORE(*primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_sir_dwrr_offset_set(uint8_t sir_dwrr_offset)
{
    if(sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_G(sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_sir_dwrr_offset_set_core(uint8_t sir_dwrr_offset, int core_id)
{
    if(sir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_WRITE_CORE(sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_sir_dwrr_offset_get(uint8_t *sir_dwrr_offset)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_G(*sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_sir_dwrr_offset_get_core(uint8_t *sir_dwrr_offset, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_SIR_DWRR_OFFSET_READ_CORE(*sir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_dwrr_offset_set(uint8_t pir_dwrr_offset)
{
    if(pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_G(pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_dwrr_offset_set_core(uint8_t pir_dwrr_offset, int core_id)
{
    if(pir_dwrr_offset >= 8)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_WRITE_CORE(pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_dwrr_offset_get(uint8_t *pir_dwrr_offset)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_G(*pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_dwrr_offset_get_core(uint8_t *pir_dwrr_offset, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_DWRR_OFFSET_READ_CORE(*pir_dwrr_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_queue_offset_set(uint16_t queue_offset)
{
    if(queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_G(queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_queue_offset_set_core(uint16_t queue_offset, int core_id)
{
    if(queue_offset >= 512)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_WRITE_CORE(queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_queue_offset_get(uint16_t *queue_offset)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_G(*queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_queue_offset_get_core(uint16_t *queue_offset, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUEUE_OFFSET_READ_CORE(*queue_offset, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_quantum_number_set(uint8_t quantum_number)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_quantum_number_set_core(uint8_t quantum_number, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_CORE(quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_quantum_number_get(uint8_t *quantum_number)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_quantum_number_get_core(uint8_t *quantum_number, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_CORE(*quantum_number, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limiter_index_set(uint8_t pir_rate_limiter_index)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_G(pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limiter_index_set_core(uint8_t pir_rate_limiter_index, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_WRITE_CORE(pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limiter_index_get(uint8_t *pir_rate_limiter_index)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_G(*pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limiter_index_get_core(uint8_t *pir_rate_limiter_index, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMITER_INDEX_READ_CORE(*pir_rate_limiter_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_slot_index_set(uint8_t primary_scheduler_slot_index)
{
    if(primary_scheduler_slot_index >= 32)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_G(primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_slot_index_set_core(uint8_t primary_scheduler_slot_index, int core_id)
{
    if(primary_scheduler_slot_index >= 32)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_WRITE_CORE(primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_slot_index_get(uint8_t *primary_scheduler_slot_index)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_G(*primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_slot_index_get_core(uint8_t *primary_scheduler_slot_index, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_SLOT_INDEX_READ_CORE(*primary_scheduler_slot_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limit_enable_set(bdmf_boolean pir_rate_limit_enable)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_G(pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limit_enable_set_core(bdmf_boolean pir_rate_limit_enable, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_WRITE_CORE(pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limit_enable_get(bdmf_boolean *pir_rate_limit_enable)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_G(*pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_pir_rate_limit_enable_get_core(bdmf_boolean *pir_rate_limit_enable, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PIR_RATE_LIMIT_ENABLE_READ_CORE(*pir_rate_limit_enable, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_index_set(uint8_t primary_scheduler_index)
{
    if(primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_G(primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_index_set_core(uint8_t primary_scheduler_index, int core_id)
{
    if(primary_scheduler_index >= 128)
          return BDMF_ERR_PARM;

    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_WRITE_CORE(primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_index_get(uint8_t *primary_scheduler_index)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_G(*primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_secondary_scheduler_table_primary_scheduler_index_get_core(uint8_t *primary_scheduler_index, int core_id)
{
    RDD_SECONDARY_SCHEDULER_DESCRIPTOR_PRIMARY_SCHEDULER_INDEX_READ_CORE(*primary_scheduler_index, RDD_SQ_TM_SECONDARY_SCHEDULER_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_start_index_set(uint32_t bits)
{
    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_SQ_TM_UPDATE_FIFO_START_INDEX_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_start_index_set_core(uint32_t bits, int core_id)
{
    RDD_BYTES_4_BITS_WRITE_CORE(bits, RDD_SQ_TM_UPDATE_FIFO_START_INDEX_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_start_index_get(uint32_t *bits)
{
    RDD_BYTES_4_BITS_READ_G(*bits, RDD_SQ_TM_UPDATE_FIFO_START_INDEX_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_start_index_get_core(uint32_t *bits, int core_id)
{
    RDD_BYTES_4_BITS_READ_CORE(*bits, RDD_SQ_TM_UPDATE_FIFO_START_INDEX_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_table_set(uint32_t _entry, bdmf_boolean valid, uint16_t pd_fifo_write_ptr, uint16_t queue_number)
{
    if(_entry >= RDD_SQ_TM_UPDATE_FIFO_TABLE_SIZE || pd_fifo_write_ptr >= 16384 || queue_number >= 512)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_WRITE_G(valid, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_WRITE_G(pd_fifo_write_ptr, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_WRITE_G(queue_number, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_table_set_core(uint32_t _entry, bdmf_boolean valid, uint16_t pd_fifo_write_ptr, uint16_t queue_number, int core_id)
{
    if(_entry >= RDD_SQ_TM_UPDATE_FIFO_TABLE_SIZE || pd_fifo_write_ptr >= 16384 || queue_number >= 512)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_WRITE_CORE(valid, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_WRITE_CORE(pd_fifo_write_ptr, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_WRITE_CORE(queue_number, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_table_get(uint32_t _entry, bdmf_boolean *valid, uint16_t *pd_fifo_write_ptr, uint16_t *queue_number)
{
    if(_entry >= RDD_SQ_TM_UPDATE_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_READ_G(*valid, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_READ_G(*pd_fifo_write_ptr, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_READ_G(*queue_number, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_update_fifo_table_get_core(uint32_t _entry, bdmf_boolean *valid, uint16_t *pd_fifo_write_ptr, uint16_t *queue_number, int core_id)
{
    if(_entry >= RDD_SQ_TM_UPDATE_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_READ_CORE(*valid, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_READ_CORE(*pd_fifo_write_ptr, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_READ_CORE(*queue_number, RDD_SQ_TM_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

