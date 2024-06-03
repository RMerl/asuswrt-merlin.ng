// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
    
*/

#include "rdd.h"


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

int                                g_dbg_lvl;
RDD_FC_MCAST_CONNECTION2_TABLE_DTS *g_fc_mcast_connection2_table_ptr;
#if defined(DSL_63138) || defined(DSL_63148)
RDD_CONNECTION_TABLE_DTS    *g_ds_connection_table_ptr;
#endif

uint8_t*  g_runner_ddr_base_addr;
uint32_t  g_runner_ddr_base_addr_phys;
uint8_t*  g_runner_tables_ptr;
uint8_t*  g_runner_extra_ddr_base_addr;
uint32_t  g_runner_extra_ddr_base_addr_phys;
uint32_t  g_ddr_headroom_size;
uint8_t   g_broadcom_switch_mode = 0;
uint32_t  g_bridge_flow_cache_mode;
uint32_t  g_cpu_tx_queue_write_ptr[ 4 ];
uint32_t  g_cpu_tx_queue_abs_data_ptr_write_ptr[ 4 ];
uint32_t  g_rate_controllers_pool_idx;
uint32_t  g_chip_revision;
uint8_t   g_lookup_port_init_mapping_table[16];

RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTE  g_ingress_classification_rule_cfg_table[ 2 ];
RDD_WAN_TX_POINTERS_TABLE_DTS                  *wan_tx_pointers_table_ptr;
BL_LILAC_RDD_BRIDGE_PORT_DTE                   g_broadcom_switch_physical_port = 0;
uint32_t  g_cpu_tx_abs_packet_limit = 0;
uint16_t  *g_free_skb_indexes_fifo_table = NULL;
uint8_t **g_cpu_tx_skb_pointers_reference_array = NULL;
uint8_t *g_dhd_tx_cpu_usage_reference_array = NULL;
rdd_phys_addr_t *g_cpu_tx_data_pointers_reference_array = NULL;
rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address = 0;
rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address_last_idx = 0;

DEFINE_BDMF_FASTLOCK( int_lock );
DEFINE_BDMF_FASTLOCK( int_lock_irq );
