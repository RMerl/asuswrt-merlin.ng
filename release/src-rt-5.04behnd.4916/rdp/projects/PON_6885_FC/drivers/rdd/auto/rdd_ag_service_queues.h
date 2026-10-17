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


#ifndef _RDD_AG_SERVICE_QUEUES_H_
#define _RDD_AG_SERVICE_QUEUES_H_

int rdd_ag_service_queues_pd_fifo_table_set(uint32_t _entry, bdmf_boolean requeue, bdmf_boolean truncated, bdmf_boolean ecn_capable, bdmf_boolean do_not_agg, bdmf_boolean multicast_first_copy, bdmf_boolean multicast_packet_copy, bdmf_boolean force_copy, uint8_t priority, bdmf_boolean color, uint16_t first_level_q, bdmf_boolean mirror_packet, uint8_t ing_port_id, uint16_t serial_num, uint8_t header_number_number_of_sbpm_bns, uint16_t second_token, bdmf_boolean ingress_multicast_3rd, bdmf_boolean dont_inc_sn, uint16_t rx_tx_sn_400g_reorder, uint16_t packet_length, bdmf_boolean err_exception, bdmf_boolean ing_mcast_copy, uint8_t disp_viq, uint8_t ing_ch, uint8_t sop, bdmf_boolean headroom_select, uint16_t bn0_first);
int rdd_ag_service_queues_pd_fifo_table_set_core(uint32_t _entry, bdmf_boolean requeue, bdmf_boolean truncated, bdmf_boolean ecn_capable, bdmf_boolean do_not_agg, bdmf_boolean multicast_first_copy, bdmf_boolean multicast_packet_copy, bdmf_boolean force_copy, uint8_t priority, bdmf_boolean color, uint16_t first_level_q, bdmf_boolean mirror_packet, uint8_t ing_port_id, uint16_t serial_num, uint8_t header_number_number_of_sbpm_bns, uint16_t second_token, bdmf_boolean ingress_multicast_3rd, bdmf_boolean dont_inc_sn, uint16_t rx_tx_sn_400g_reorder, uint16_t packet_length, bdmf_boolean err_exception, bdmf_boolean ing_mcast_copy, uint8_t disp_viq, uint8_t ing_ch, uint8_t sop, bdmf_boolean headroom_select, uint16_t bn0_first, int core_id);
int rdd_ag_service_queues_pd_fifo_table_get(uint32_t _entry, bdmf_boolean *requeue, bdmf_boolean *truncated, bdmf_boolean *ecn_capable, bdmf_boolean *do_not_agg, bdmf_boolean *multicast_first_copy, bdmf_boolean *multicast_packet_copy, bdmf_boolean *force_copy, uint8_t *priority, bdmf_boolean *color, uint16_t *first_level_q, bdmf_boolean *mirror_packet, uint8_t *ing_port_id, uint16_t *serial_num, uint8_t *header_number_number_of_sbpm_bns, uint16_t *second_token, bdmf_boolean *ingress_multicast_3rd, bdmf_boolean *dont_inc_sn, uint16_t *rx_tx_sn_400g_reorder, uint16_t *packet_length, bdmf_boolean *err_exception, bdmf_boolean *ing_mcast_copy, uint8_t *disp_viq, uint8_t *ing_ch, uint8_t *sop, bdmf_boolean *headroom_select, uint16_t *bn0_first);
int rdd_ag_service_queues_pd_fifo_table_get_core(uint32_t _entry, bdmf_boolean *requeue, bdmf_boolean *truncated, bdmf_boolean *ecn_capable, bdmf_boolean *do_not_agg, bdmf_boolean *multicast_first_copy, bdmf_boolean *multicast_packet_copy, bdmf_boolean *force_copy, uint8_t *priority, bdmf_boolean *color, uint16_t *first_level_q, bdmf_boolean *mirror_packet, uint8_t *ing_port_id, uint16_t *serial_num, uint8_t *header_number_number_of_sbpm_bns, uint16_t *second_token, bdmf_boolean *ingress_multicast_3rd, bdmf_boolean *dont_inc_sn, uint16_t *rx_tx_sn_400g_reorder, uint16_t *packet_length, bdmf_boolean *err_exception, bdmf_boolean *ing_mcast_copy, uint8_t *disp_viq, uint8_t *ing_ch, uint8_t *sop, bdmf_boolean *headroom_select, uint16_t *bn0_first, int core_id);
int rdd_ag_service_queues_update_fifo_table_set(uint32_t _entry, bdmf_boolean valid, uint16_t pd_fifo_write_ptr, uint16_t queue_number);
int rdd_ag_service_queues_update_fifo_table_set_core(uint32_t _entry, bdmf_boolean valid, uint16_t pd_fifo_write_ptr, uint16_t queue_number, int core_id);
int rdd_ag_service_queues_update_fifo_table_get(uint32_t _entry, bdmf_boolean *valid, uint16_t *pd_fifo_write_ptr, uint16_t *queue_number);
int rdd_ag_service_queues_update_fifo_table_get_core(uint32_t _entry, bdmf_boolean *valid, uint16_t *pd_fifo_write_ptr, uint16_t *queue_number, int core_id);

#endif /* _RDD_AG_SERVICE_QUEUES_H_ */
