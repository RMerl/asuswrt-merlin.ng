/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :> 
 */


#ifndef _TC_TO_Q_H
#define _TC_TO_Q_H

#include "rdd.h"
#include "rdd_defs.h"

#define RDPA_CS_TC_TO_QUEUE_TABLE_SIZE 32
#define RDPA_BS_TC_TO_QUEUE_TABLE_SIZE 8
/* TODO: maybe ifdef on not G9991 RDD_DS_QOS_MAPPER_ID_MAX_TABLES = 8*/
#define RDD_DS_QOS_MAPPER_ID_MAX_TABLES   32    /* Max number of TC-to-QM_QUEUE DS mapping tables. */
#define RDD_US_QOS_MAPPER_ID_MAX_TABLES   32    /* Max number of TC-to-QM_QUEUE US mapping tables. */
/* The last allocated TC2Q table is used as an invalid table and cannot be used */
#define RDD_QOS_MAPPER_ID_MAX_TABLES      (RDD_TC_TO_QUEUE_TABLE_SIZE - 1)
#define RDD_QOS_MAPPER_INVALID_TABLE       RDD_TC_TO_QUEUE_TABLE_SIZE

/* API to RDPA level */
bdmf_error_t rdd_tc_to_queue_entry_set(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, 
    uint8_t tc, uint16_t qm_queue_index, bdmf_boolean is_tcont, bdmf_number tcont_idx);
bdmf_error_t rdd_realloc_tc_to_queue_table(uint16_t port, rdpa_traffic_dir dir, uint8_t *size);
void rdd_qos_mapper_invalidate_table(uint16_t port_or_wan_flow, rdpa_traffic_dir dir,
    uint8_t size, bdmf_boolean is_pbit);
/* Internal RDD functions */
void rdd_qos_mapper_init(void);
bdmf_error_t rdd_pbit_to_queue_entry_set(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, uint8_t pbit, 
    uint16_t qm_queue_index,  bdmf_boolean is_tcont, bdmf_number tcont_idx);
bdmf_error_t rdd_us_pbits_to_wan_flow_entry_cfg(uint8_t gem_mapping_table, uint8_t pbit, uint8_t gem);
void rdd_qos_mapper_set_table_id_to_tx_flow(uint16_t src_tx_flow, uint16_t dst_tx_flow);

#endif




