/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :> 
 */


#ifndef _TC_TO_Q_H
#define _TC_TO_Q_H

#include "rdd.h"
#include "rdd_defs.h"

#define RDPA_CS_TC_TO_QUEUE_TABLE_SIZE 32
#define RDPA_BS_TC_TO_QUEUE_TABLE_SIZE 8

/* The last allocated TC2Q table is used as an invalid table and cannot be used */
#define RDD_QOS_MAPPER_ID_MAX_TABLES      (RDD_TC_TO_QUEUE_TABLE_SIZE - 1)
#define RDD_QOS_MAPPER_INVALID_TABLE       RDD_TC_TO_QUEUE_TABLE_SIZE

/* TODO: maybe ifdef on not G9991 RDD_DS_QOS_MAPPER_ID_MAX_TABLES = 8*/
#define RDD_DS_QOS_MAPPER_ID_MAX_TABLES   (RDD_QOS_MAPPER_ID_MAX_TABLES/2)    /* Max number of TC-to-QM_QUEUE DS mapping tables. */
#define RDD_US_QOS_MAPPER_ID_MAX_TABLES   (RDD_QOS_MAPPER_ID_MAX_TABLES/2)    /* Max number of TC-to-QM_QUEUE US mapping tables. */

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




