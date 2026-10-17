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

int rdd_ag_service_queues_pd_fifo_table_set(uint32_t _entry, bdmf_boolean requeue, bdmf_boolean truncated, bdmf_boolean ecn_capable, bdmf_boolean do_not_agg, bdmf_boolean multicast_first_copy, bdmf_boolean multicast_packet_copy, bdmf_boolean force_copy, uint8_t priority, bdmf_boolean color, uint16_t first_level_q, bdmf_boolean mirror_packet, uint8_t ing_port_id, uint16_t serial_num, uint8_t header_number_number_of_sbpm_bns, uint16_t second_token, bdmf_boolean ingress_multicast_3rd, bdmf_boolean dont_inc_sn, uint16_t rx_tx_sn_400g_reorder, uint16_t packet_length, bdmf_boolean err_exception, bdmf_boolean ing_mcast_copy, uint8_t disp_viq, uint8_t ing_ch, uint8_t sop, bdmf_boolean headroom_select, uint16_t bn0_first)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE || priority >= 4 || first_level_q >= 32768 || ing_port_id >= 64 || serial_num >= 2048 || header_number_number_of_sbpm_bns >= 128 || second_token >= 16384 || packet_length >= 16384 || disp_viq >= 16 || ing_ch >= 16 || sop >= 128 || bn0_first >= 16384)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_REQUEUE_WRITE_G(requeue, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TRUNCATED_WRITE_G(truncated, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ECN_CAPABLE_WRITE_G(ecn_capable, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DO_NOT_AGG_WRITE_G(do_not_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_FIRST_COPY_WRITE_G(multicast_first_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_PACKET_COPY_WRITE_G(multicast_packet_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE_G(force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE_G(priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE_G(color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_G(first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MIRROR_PACKET_WRITE_G(mirror_packet, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_PORT_ID_WRITE_G(ing_port_id, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE_G(serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADER_NUMBER_NUMBER_OF_SBPM_BNS_WRITE_G(header_number_number_of_sbpm_bns, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_TOKEN_WRITE_G(second_token, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_MULTICAST_3RD_WRITE_G(ingress_multicast_3rd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_INC_SN_WRITE_G(dont_inc_sn, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_RX_TX_SN_400G_REORDER_WRITE_G(rx_tx_sn_400g_reorder, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_G(packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ERR_EXCEPTION_WRITE_G(err_exception, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_MCAST_COPY_WRITE_G(ing_mcast_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DISP_VIQ_WRITE_G(disp_viq, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_CH_WRITE_G(ing_ch, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SOP_WRITE_G(sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_SELECT_WRITE_G(headroom_select, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_WRITE_G(bn0_first, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_pd_fifo_table_set_core(uint32_t _entry, bdmf_boolean requeue, bdmf_boolean truncated, bdmf_boolean ecn_capable, bdmf_boolean do_not_agg, bdmf_boolean multicast_first_copy, bdmf_boolean multicast_packet_copy, bdmf_boolean force_copy, uint8_t priority, bdmf_boolean color, uint16_t first_level_q, bdmf_boolean mirror_packet, uint8_t ing_port_id, uint16_t serial_num, uint8_t header_number_number_of_sbpm_bns, uint16_t second_token, bdmf_boolean ingress_multicast_3rd, bdmf_boolean dont_inc_sn, uint16_t rx_tx_sn_400g_reorder, uint16_t packet_length, bdmf_boolean err_exception, bdmf_boolean ing_mcast_copy, uint8_t disp_viq, uint8_t ing_ch, uint8_t sop, bdmf_boolean headroom_select, uint16_t bn0_first, int core_id)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE || priority >= 4 || first_level_q >= 32768 || ing_port_id >= 64 || serial_num >= 2048 || header_number_number_of_sbpm_bns >= 128 || second_token >= 16384 || packet_length >= 16384 || disp_viq >= 16 || ing_ch >= 16 || sop >= 128 || bn0_first >= 16384)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_REQUEUE_WRITE_CORE(requeue, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TRUNCATED_WRITE_CORE(truncated, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ECN_CAPABLE_WRITE_CORE(ecn_capable, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DO_NOT_AGG_WRITE_CORE(do_not_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_FIRST_COPY_WRITE_CORE(multicast_first_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_PACKET_COPY_WRITE_CORE(multicast_packet_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE_CORE(force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE_CORE(priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE_CORE(color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_CORE(first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MIRROR_PACKET_WRITE_CORE(mirror_packet, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_PORT_ID_WRITE_CORE(ing_port_id, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE_CORE(serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADER_NUMBER_NUMBER_OF_SBPM_BNS_WRITE_CORE(header_number_number_of_sbpm_bns, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_TOKEN_WRITE_CORE(second_token, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_MULTICAST_3RD_WRITE_CORE(ingress_multicast_3rd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_INC_SN_WRITE_CORE(dont_inc_sn, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_RX_TX_SN_400G_REORDER_WRITE_CORE(rx_tx_sn_400g_reorder, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_CORE(packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ERR_EXCEPTION_WRITE_CORE(err_exception, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_MCAST_COPY_WRITE_CORE(ing_mcast_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DISP_VIQ_WRITE_CORE(disp_viq, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_CH_WRITE_CORE(ing_ch, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SOP_WRITE_CORE(sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_SELECT_WRITE_CORE(headroom_select, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_WRITE_CORE(bn0_first, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_pd_fifo_table_get(uint32_t _entry, bdmf_boolean *requeue, bdmf_boolean *truncated, bdmf_boolean *ecn_capable, bdmf_boolean *do_not_agg, bdmf_boolean *multicast_first_copy, bdmf_boolean *multicast_packet_copy, bdmf_boolean *force_copy, uint8_t *priority, bdmf_boolean *color, uint16_t *first_level_q, bdmf_boolean *mirror_packet, uint8_t *ing_port_id, uint16_t *serial_num, uint8_t *header_number_number_of_sbpm_bns, uint16_t *second_token, bdmf_boolean *ingress_multicast_3rd, bdmf_boolean *dont_inc_sn, uint16_t *rx_tx_sn_400g_reorder, uint16_t *packet_length, bdmf_boolean *err_exception, bdmf_boolean *ing_mcast_copy, uint8_t *disp_viq, uint8_t *ing_ch, uint8_t *sop, bdmf_boolean *headroom_select, uint16_t *bn0_first)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_REQUEUE_READ_G(*requeue, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_TRUNCATED_READ_G(*truncated, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ECN_CAPABLE_READ_G(*ecn_capable, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DO_NOT_AGG_READ_G(*do_not_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_FIRST_COPY_READ_G(*multicast_first_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_PACKET_COPY_READ_G(*multicast_packet_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ_G(*force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ_G(*priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ_G(*color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_G(*first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_MIRROR_PACKET_READ_G(*mirror_packet, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_PORT_ID_READ_G(*ing_port_id, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ_G(*serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADER_NUMBER_NUMBER_OF_SBPM_BNS_READ_G(*header_number_number_of_sbpm_bns, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_TOKEN_READ_G(*second_token, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_MULTICAST_3RD_READ_G(*ingress_multicast_3rd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_INC_SN_READ_G(*dont_inc_sn, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_RX_TX_SN_400G_REORDER_READ_G(*rx_tx_sn_400g_reorder, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ_G(*packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ERR_EXCEPTION_READ_G(*err_exception, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_MCAST_COPY_READ_G(*ing_mcast_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_DISP_VIQ_READ_G(*disp_viq, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_CH_READ_G(*ing_ch, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_SOP_READ_G(*sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_SELECT_READ_G(*headroom_select, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_READ_G(*bn0_first, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_service_queues_pd_fifo_table_get_core(uint32_t _entry, bdmf_boolean *requeue, bdmf_boolean *truncated, bdmf_boolean *ecn_capable, bdmf_boolean *do_not_agg, bdmf_boolean *multicast_first_copy, bdmf_boolean *multicast_packet_copy, bdmf_boolean *force_copy, uint8_t *priority, bdmf_boolean *color, uint16_t *first_level_q, bdmf_boolean *mirror_packet, uint8_t *ing_port_id, uint16_t *serial_num, uint8_t *header_number_number_of_sbpm_bns, uint16_t *second_token, bdmf_boolean *ingress_multicast_3rd, bdmf_boolean *dont_inc_sn, uint16_t *rx_tx_sn_400g_reorder, uint16_t *packet_length, bdmf_boolean *err_exception, bdmf_boolean *ing_mcast_copy, uint8_t *disp_viq, uint8_t *ing_ch, uint8_t *sop, bdmf_boolean *headroom_select, uint16_t *bn0_first, int core_id)
{
    if(_entry >= RDD_SQ_TM_PD_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_PROCESSING_TX_DESCRIPTOR_REQUEUE_READ_CORE(*requeue, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_TRUNCATED_READ_CORE(*truncated, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ECN_CAPABLE_READ_CORE(*ecn_capable, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DO_NOT_AGG_READ_CORE(*do_not_agg, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_FIRST_COPY_READ_CORE(*multicast_first_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MULTICAST_PACKET_COPY_READ_CORE(*multicast_packet_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ_CORE(*force_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ_CORE(*priority, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ_CORE(*color, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_CORE(*first_level_q, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_MIRROR_PACKET_READ_CORE(*mirror_packet, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_PORT_ID_READ_CORE(*ing_port_id, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ_CORE(*serial_num, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADER_NUMBER_NUMBER_OF_SBPM_BNS_READ_CORE(*header_number_number_of_sbpm_bns, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SECOND_TOKEN_READ_CORE(*second_token, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_MULTICAST_3RD_READ_CORE(*ingress_multicast_3rd, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DONT_INC_SN_READ_CORE(*dont_inc_sn, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_RX_TX_SN_400G_REORDER_READ_CORE(*rx_tx_sn_400g_reorder, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ_CORE(*packet_length, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ERR_EXCEPTION_READ_CORE(*err_exception, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_MCAST_COPY_READ_CORE(*ing_mcast_copy, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_DISP_VIQ_READ_CORE(*disp_viq, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_ING_CH_READ_CORE(*ing_ch, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_SOP_READ_CORE(*sop, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_SELECT_READ_CORE(*headroom_select, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_READ_CORE(*bn0_first, RDD_SQ_TM_PD_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

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

