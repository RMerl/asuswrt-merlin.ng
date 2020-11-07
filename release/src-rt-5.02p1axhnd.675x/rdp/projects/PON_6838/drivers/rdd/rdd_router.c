/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
       All Rights Reserved
    
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

#include "rdd.h"


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

#if defined(FIRMWARE_INIT)
extern uint32_t  DsConnectionTableBase;
extern uint32_t  UsConnectionTableBase;
extern uint32_t  ContextTableBase;
extern uint32_t  FirewallRulesMapTable;
extern uint32_t  FirewallRulesTable;
#endif

extern uint8_t  *g_runner_ddr_base_addr;
extern uint32_t  g_runner_tables_ptr;
extern uint32_t  g_free_context_entries_number;
extern uint32_t  g_free_context_entries_head;
extern uint32_t  g_free_context_entries_tail;
extern uint32_t  *g_free_connection_context_entries;
extern RDD_64_BIT_TABLE_CFG  g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
extern RDD_DDR_TABLE_CFG     g_ddr_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
void f_global_ingress_vector_config ( uint32_t, rdpa_traffic_dir, bdmf_boolean );

extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_DTE    f_rdd_lock;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_DTE  f_rdd_unlock;
extern bdmf_fastlock                                int_lock;

extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_IRQ_DTE    f_rdd_lock_irq;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_IRQ_DTE  f_rdd_unlock_irq;
extern bdmf_fastlock                                    int_lock_irq;

BL_LILAC_RDD_ERROR_DTE f_rdd_layer4_filters_initialize ( void )
{
    RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_DTS   *ds_layer4_filter_table_ptr;
    RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_DTS   *us_layer4_filter_table_ptr;
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DTS      *layer4_filter_entry_ptr;
    RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_DTS  *ds_layer4_filter_context_table_ptr;
    RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_DTS  *us_layer4_filter_context_table_ptr;
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DTS     *layer4_filter_context_entry_ptr;
    RUNNER_REGS_CFG_CAM_CFG                  runner_cam_configuration_register;

    runner_cam_configuration_register.stop_value = 0xFFFF;

    RUNNER_REGS_0_CFG_CAM_CFG_WRITE ( runner_cam_configuration_register );
    RUNNER_REGS_1_CFG_CAM_CFG_WRITE ( runner_cam_configuration_register );

    /* Downstream layer 4 filters */
    ds_layer4_filter_table_ptr = RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_PTR();

    ds_layer4_filter_context_table_ptr = RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_PTR();

    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_ERROR ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_ERROR ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_EXCEPTION ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_EXCEPTION ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FIRST_FRAGMENT ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FIRST_FRAGMENT ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FRAGMENT ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FRAGMENT ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_ICMP ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_ICMP ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ICMP, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_ESP ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_ESP ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ESP, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_GRE ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_GRE ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_GRE, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV4, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE ( 0x3, layer4_filter_entry_ptr );

    /* disable RDD_LAYER4_FILTER_L3_IPV4 at initialization */
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0x0, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV6, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE ( 0x3, layer4_filter_entry_ptr );

    /* disable LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV6 at initialization */
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0x0, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_AH ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_AH ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_AH, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_IPV6 ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_IPV6 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_IPV6, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_0 ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_0 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_0, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_1 ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_1 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_1, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_2 ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_2 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_2, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_3 ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_3 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_3, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_UNKNOWN ] );
    layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_UNKNOWN ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_UNKNOWN, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0x0, layer4_filter_entry_ptr );

    /* Upstream layer 4 filters */
    us_layer4_filter_table_ptr = RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_PTR();

    us_layer4_filter_context_table_ptr = RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_PTR();

    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_ERROR ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_ERROR ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_EXCEPTION ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_EXCEPTION ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FIRST_FRAGMENT ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FIRST_FRAGMENT ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FRAGMENT ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_IP_FRAGMENT ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_WRITE ( LILAC_RDD_ON, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_MASK_WRITE ( 0x1, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_ICMP ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_ICMP ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ICMP, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_ESP ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_ESP ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ESP, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_GRE ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_GRE ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_GRE, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV4, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE ( 0x3, layer4_filter_entry_ptr );

    /* disable RDD_LAYER4_FILTER_L3_IPV4 at initialization  */
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0x0, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV6, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE ( 0x3, layer4_filter_entry_ptr );

    /* disable LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV6 at initialization  */
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0x0, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_AH ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_AH ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_AH, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_IPV6 ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_IPV6 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_IPV6, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_0 ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_0 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_0, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_1 ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_1 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_1, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_2 ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_2 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_2, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_3 ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_USER_DEFINED_3 ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_3, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, layer4_filter_entry_ptr );


    layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_UNKNOWN ] );
    layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ RDD_LAYER4_FILTER_UNKNOWN ] );

    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_non_tcp_udp, layer4_filter_context_entry_ptr );

    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_UNKNOWN, layer4_filter_entry_ptr );
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0x0, layer4_filter_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_free_context_entry ( uint32_t  context_entry_index )
{
    g_free_connection_context_entries [ g_free_context_entries_tail++ ] = context_entry_index;
    g_free_context_entries_tail = g_free_context_entries_tail % RDD_CONTEXT_TABLE_SIZE;
    g_free_context_entries_number++;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_connection_table_initialize ( void )
{
    uint32_t  *connection_table_config_ptr;
    uint32_t  connection_table_address;
    uint32_t  *context_table_config_ptr;
    uint32_t  context_table_address;
    uint32_t  i;

    connection_table_address = ( g_runner_tables_ptr + DS_CONNECTION_TABLE_ADDRESS ) & 0x1FFFFFFF;
    connection_table_config_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CONNECTION_TABLE_CONFIG_ADDRESS );
    MWRITE_32( connection_table_config_ptr,  connection_table_address );

    connection_table_address = ( g_runner_tables_ptr + US_CONNECTION_TABLE_ADDRESS ) & 0x1FFFFFFF;
    connection_table_config_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CONNECTION_TABLE_CONFIG_ADDRESS );
    MWRITE_32( connection_table_config_ptr,  connection_table_address );

    context_table_address = ( g_runner_tables_ptr + CONTEXT_TABLE_ADDRESS ) & 0x1FFFFFFF;
    context_table_config_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CONTEXT_TABLE_CONFIG_ADDRESS );
    MWRITE_32( context_table_config_ptr, context_table_address );
    context_table_config_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CONTEXT_TABLE_CONFIG_ADDRESS );
    MWRITE_32( context_table_config_ptr, context_table_address );

    g_free_connection_context_entries = ( uint32_t *)malloc( RDD_CONTEXT_TABLE_SIZE * sizeof ( uint32_t ) );
    g_free_context_entries_number = 0;
    g_free_context_entries_head = 0;
    g_free_context_entries_tail = 0;

    for ( i = 0; i < RDD_CONTEXT_TABLE_SIZE; i++ )
    {
        f_rdd_free_context_entry ( i );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_subnet_classify_config ( BL_LILAC_RDD_SUBNET_CLASSIFY_MODE_DTE  xi_subnet_classify_mode )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_SUBNET_CLASSIFICATION_MODE_WRITE ( xi_subnet_classify_mode, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ipv6_config ( BL_LILAC_RDD_IPV6_ENABLE_DTE  xi_ipv6_mode )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ENABLE_WRITE ( xi_ipv6_mode, bridge_cfg_register );

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ENABLE_WRITE ( xi_ipv6_mode, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}

void _rdd_fc_context_read_from_table_entry(rdd_fc_context_t *ctx, RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry)
{
    int i;
    uint8_t wifi_ssid;
    uint16_t wl_metadata = 0;

    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ(ctx->conn_dir, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ(ctx->conn_index, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIONS_VECTOR_READ(ctx->actions_vector, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_FWD_ACTION_READ(ctx->fwd_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CPU_REASON_READ(ctx->trap_reason, entry);

    ctx->trap_reason = ctx->trap_reason + rdpa_cpu_rx_reason_udef_0;

    RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUES_MODE_READ(ctx->service_queue_enabled, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_READ(ctx->service_queue_id, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ(ctx->qos_method, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_VERSION_READ(ctx->ip_version, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_PORT_READ(ctx->nat_port, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_IP_READ(ctx->nat_ip.addr.ipv4, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_VID_OFFSET_READ(ctx->ovid_offset, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_PBIT_REMAP_ACTION_READ(ctx->opbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_INNER_PBIT_REMAP_ACTION_READ(ctx->ipbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DSCP_VALUE_READ(ctx->dscp_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ECN_VALUE_READ(ctx->ecn_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_POLICER_ID_READ(ctx->policer_id, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_EGRESS_PORT_READ(ctx->egress_port, entry);
    if (ctx->egress_port == BL_LILAC_RDD_PCI_BRIDGE_PORT)
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_IS_WFD_READ(ctx->wfd.nic_ucast.is_wfd, entry);
        if (ctx->wfd.nic_ucast.is_wfd)
        {
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_IDX_READ(ctx->wfd.nic_ucast.wfd_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_PRIO_READ(ctx->wfd.nic_ucast.wfd_prio, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_READ(wl_metadata, entry);
            
            if (wl_metadata & (1 << 13))
            {
                ctx->wfd.nic_ucast.is_chain = 1;
                ctx->wfd.nic_ucast.priority = (wl_metadata & 0xf00) >> 8;
                ctx->wfd.nic_ucast.chain_idx = wl_metadata & 0xff;
            }
            else
            {
                ctx->wfd.dhd_ucast.is_chain = 0;
                ctx->wfd.dhd_ucast.priority = (wl_metadata & 0x1c00) >> 10;
                ctx->wfd.dhd_ucast.flowring_idx = wl_metadata & 0x3ff;
            }
        }
        else
        {
            RDD_FLOW_CACHE_CONTEXT_ENTRY_PRIORITY_READ(ctx->rnr.priority, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_RING_ID_READ(ctx->rnr.flowring_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_RADIO_IDX_READ(ctx->rnr.radio_idx, entry);
        }
    }
    else if (ctx->egress_port == BL_LILAC_RDD_CPU_BRIDGE_PORT)
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_READ(wl_metadata, entry);
        ctx->wfd.nic_ucast.is_wfd = 1;
        ctx->wfd.nic_ucast.is_chain = 1;
        ctx->wfd.nic_ucast.chain_idx = wl_metadata & 0xff;
    }
    else
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAFFIC_CLASS_READ(ctx->traffic_class, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_WAN_FLOW_INDEX_READ(ctx->wan_flow_index, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_RATE_CONTROLLER_READ(ctx->rate_controller, entry);
    }

    RDD_FLOW_CACHE_CONTEXT_ENTRY_WIFI_SSID_READ(wifi_ssid, entry);
    if (ctx->egress_port == BL_LILAC_RDD_PCI_BRIDGE_PORT && !ctx->wfd.nic_ucast.is_wfd)
    {
        /* In DHD TX Post descriptor, wifi_ssid expected to be subunit */
        ctx->wifi_ssid = wifi_ssid + ctx->rnr.radio_idx * WL_NUM_OF_SSID_PER_UNIT;
    }
    else
    {
        ctx->wifi_ssid = 0;
    }

    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_OFFSET_READ(ctx->l2_hdr_offset, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_SIZE_READ(ctx->l2_hdr_size, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER2_HEADER_NUMBER_OF_TAGS_READ(ctx->l2_hdr_number_of_tags, entry);
    for (i = 0; i < RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER; i++)
        RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_READ(ctx->l2_header[i], entry, i);
    
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_PACKETS_COUNTER_READ(ctx->valid_cnt.packets, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_BYTES_COUNTER_READ(ctx->valid_cnt.bytes, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DROP_ELIGIBILITY_READ(ctx->drop_eligibility, entry);
}

static void _rdd_fc_context_write_to_table_entry(rdd_fc_context_t *ctx, RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry,
    bdmf_boolean is_add)
{
    int i;
    uint8_t wifi_ssid;
    
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_WRITE(ctx->conn_dir, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_WRITE(ctx->conn_index, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIONS_VECTOR_WRITE(ctx->actions_vector, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_FWD_ACTION_WRITE(ctx->fwd_action, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUES_MODE_WRITE(ctx->service_queue_enabled, entry);
    
    if (( ctx->actions_vector & rdpa_fc_action_forward ) && ( ctx->fwd_action == RDD_FLOW_CACHE_FORWARD_ACTION_CPU))
    {
        ctx->trap_reason = ctx->trap_reason - rdpa_cpu_rx_reason_udef_0;
        RDD_FLOW_CACHE_CONTEXT_ENTRY_CPU_REASON_WRITE(ctx->trap_reason, entry);
    }
    else
    {
    	RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_WRITE(ctx->service_queue_id, entry);
    }
    	
    RDD_FLOW_CACHE_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE(ctx->qos_method, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_VERSION_WRITE(ctx->ip_version, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_PORT_WRITE(ctx->nat_port, entry);
    /* We modify NAT IP only when we add new entry */
    if (is_add && ctx->ip_version == bdmf_ip_family_ipv4)
        RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_IP_WRITE(ctx->nat_ip.addr.ipv4, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_VID_OFFSET_WRITE(ctx->ovid_offset, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_PBIT_REMAP_ACTION_WRITE(ctx->opbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_INNER_PBIT_REMAP_ACTION_WRITE(ctx->ipbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DSCP_VALUE_WRITE(ctx->dscp_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ECN_VALUE_WRITE(ctx->ecn_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_POLICER_ID_WRITE(ctx->policer_id, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_EGRESS_PORT_WRITE(ctx->egress_port, entry);
    wifi_ssid = ctx->wifi_ssid;
    if (ctx->egress_port == BL_LILAC_RDD_PCI_BRIDGE_PORT)
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_IS_WFD_WRITE(ctx->wfd.nic_ucast.is_wfd, entry);
        if (ctx->wfd.nic_ucast.is_wfd)
        {
            uint16_t wl_metadata = 0;
            /* In the nic mode and in the dhd this fields are on the same places so we could 
               just use one of the union */
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_IDX_WRITE(ctx->wfd.nic_ucast.wfd_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_PRIO_WRITE(ctx->wfd.nic_ucast.wfd_prio, entry);
            /* XXX: Refactor to include metadata of 32b */
            if (ctx->wfd.nic_ucast.is_chain)
            {
                wl_metadata = (1 << 13) | (ctx->wfd.nic_ucast.chain_idx & 0xff); /* XXX: Temporary, until refactored */
                wl_metadata |= ctx->wfd.nic_ucast.priority << 8;
            }
            else
            {
                wl_metadata = ctx->wfd.dhd_ucast.flowring_idx;
                wl_metadata |= ctx->wfd.dhd_ucast.priority << 10;

            }

            RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_WRITE(wl_metadata, entry);
        }
        else
        {
            RDD_FLOW_CACHE_CONTEXT_ENTRY_PRIORITY_WRITE(ctx->rnr.priority, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_RING_ID_WRITE(ctx->rnr.flowring_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_RADIO_IDX_WRITE(ctx->rnr.radio_idx, entry);
            /* In DHD TX Post descriptor, wifi_ssid expected to be subunit */
            wifi_ssid %= WL_NUM_OF_SSID_PER_UNIT;

        }
    }
    else if (ctx->egress_port == BL_LILAC_RDD_CPU_BRIDGE_PORT)
    {
        uint16_t wl_metadata = 0;
        wl_metadata = (1 << 13) | (ctx->wfd.nic_ucast.chain_idx & 0xff); /* XXX: Temporary, until refactored */
        RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_WRITE(wl_metadata, entry);
    }
    else
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE(ctx->traffic_class, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_WAN_FLOW_INDEX_WRITE(ctx->wan_flow_index, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_RATE_CONTROLLER_WRITE(ctx->rate_controller, entry);
    }

    RDD_FLOW_CACHE_CONTEXT_ENTRY_WIFI_SSID_WRITE(wifi_ssid, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_OFFSET_WRITE(ctx->l2_hdr_offset, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_SIZE_WRITE(ctx->l2_hdr_size, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER2_HEADER_NUMBER_OF_TAGS_WRITE(ctx->l2_hdr_number_of_tags, entry);
    for (i = 0; i < RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER; i++)
        RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_WRITE(ctx->l2_header[i], entry, i);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DROP_ELIGIBILITY_WRITE(ctx->drop_eligibility, entry);
}

static BL_LILAC_RDD_ERROR_DTE _rdd_context_entry_add(rdd_fc_context_t *context_entry, uint32_t *context_entry_index,
    uint32_t ip_checksum_delta, uint32_t l4_checksum_delta)
{
    RDD_CONTEXT_TABLE_DTS *context_table_ptr;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *context_entry_ptr;

    context_table_ptr = (RDD_CONTEXT_TABLE_DTS *)ContextTableBase;

    if (g_free_context_entries_number > LILAC_RDD_RESERVED_CONTEXT_ENTRIES)
    {
        *context_entry_index = g_free_connection_context_entries[g_free_context_entries_head++];
        g_free_context_entries_head = g_free_context_entries_head % RDD_CONTEXT_TABLE_SIZE;
        g_free_context_entries_number--;
    }
    else
    {
        return BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY;
    }

    context_entry_ptr = &(context_table_ptr->entry[*context_entry_index]);

    _rdd_fc_context_write_to_table_entry(context_entry, context_entry_ptr, 1);
    /* Need for add only */
    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_CHECKSUM_DELTA_WRITE(ip_checksum_delta, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER4_CHECKSUM_DELTA_WRITE(l4_checksum_delta, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OVERFLOW_WRITE(0, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_PACKETS_COUNTER_WRITE(0, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_BYTES_COUNTER_WRITE(0, context_entry_ptr);

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_calculate_checksum_delta ( uint16_t  xi_checksum_delta,
                                                               uint16_t  xi_old_value,
                                                               uint16_t  xi_new_value,
                                                               uint16_t  *xo_checksum_delta )
{
    uint32_t  calculated_delta;

    calculated_delta = ( xi_checksum_delta ) + ( ( ~xi_old_value ) & 0xFFFF );
    calculated_delta += ( ( calculated_delta >> 16 ) & 0xFFFF );
    calculated_delta &= 0xFFFF;

    calculated_delta += ( xi_new_value );
    calculated_delta += ( ( calculated_delta >> 16 ) & 0xFFFF );
    calculated_delta &= 0xFFFF;

    *xo_checksum_delta = calculated_delta;

    return ( BL_LILAC_RDD_OK );
}

static void _rdd_calculate_connection_delta(BL_LILAC_RDD_ADD_CONNECTION_DTE *connection,
    rdpa_traffic_dir dir, uint32_t *ip_checksum_delta, uint32_t *l4_checksum_delta)
{
    uint16_t calculated_delta;
    uint32_t nonat_ipv4_addr;
    uint16_t nonat_port;

    if (dir == rdpa_dir_ds)
    {
        nonat_ipv4_addr = connection->lookup_entry->dst_ip.addr.ipv4;
        nonat_port = connection->lookup_entry->dst_port;
    }
    else
    {
        nonat_ipv4_addr = connection->lookup_entry->src_ip.addr.ipv4;
        nonat_port = connection->lookup_entry->src_port; 
    }

    /* IP checksum delta */
    f_rdd_calculate_checksum_delta(0, nonat_ipv4_addr, connection->context_entry.nat_ip.addr.ipv4, &calculated_delta);
    f_rdd_calculate_checksum_delta(calculated_delta, (nonat_ipv4_addr >> 16),
            (connection->context_entry.nat_ip.addr.ipv4 >> 16), &calculated_delta);
    *ip_checksum_delta = calculated_delta;

    /* TCP/UDP checksum delta */
    f_rdd_calculate_checksum_delta(calculated_delta, nonat_port, connection->context_entry.nat_port, &calculated_delta);
    *l4_checksum_delta = calculated_delta;
}

static inline BL_LILAC_RDD_ERROR_DTE f_rdd_connection_entry_alloc ( uint32_t                  xi_hash_index,
                                                                    RDD_CONNECTION_TABLE_DTS  *xi_connection_table_ptr,
                                                                    rdpa_ip_flow_key_t        *xi_lookup_entry,
                                                                    uint32_t                  xi_ipv6_src_ip_crc,
                                                                    uint32_t                  xi_ipv6_dst_ip_crc,
                                                                    uint32_t                  *xo_tries )
{
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  tries;
    uint32_t                  connection_entry_index;
    uint32_t                  connection_entry_valid;
    uint8_t                   connection_entry_protocol;
    uint16_t                  connection_entry_src_port;
    uint16_t                  connection_entry_dst_port;
    bdmf_ip_t                 connection_entry_src_ip;
    bdmf_ip_t                 connection_entry_dst_ip;

    for ( tries = 0; tries < LILAC_RDD_CONNECTION_TABLE_SET_SIZE; tries++ )
    {
        connection_entry_index = xi_hash_index + tries;

        connection_entry_ptr = &( xi_connection_table_ptr->entry[ connection_entry_index ] );

        RDD_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );

        if ( !( connection_entry_valid ) )
        {
            break;
        }

        /* if entry is valid, check if it matches entry being added */
        RDD_CONNECTION_ENTRY_PROTOCOL_READ ( connection_entry_protocol, connection_entry_ptr );
        RDD_CONNECTION_ENTRY_SRC_PORT_READ ( connection_entry_src_port, connection_entry_ptr );
        RDD_CONNECTION_ENTRY_DST_PORT_READ ( connection_entry_dst_port, connection_entry_ptr );
        RDD_CONNECTION_ENTRY_SRC_IP_READ ( connection_entry_src_ip.addr.ipv4, connection_entry_ptr );
        RDD_CONNECTION_ENTRY_DST_IP_READ ( connection_entry_dst_ip.addr.ipv4, connection_entry_ptr );

        if ( ( connection_entry_protocol == xi_lookup_entry->prot ) &&
             ( connection_entry_src_port == xi_lookup_entry->src_port ) &&
             ( connection_entry_dst_port == xi_lookup_entry->dst_port ) &&
             ( ( ( xi_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 ) &&
                 ( connection_entry_src_ip.addr.ipv4 == xi_lookup_entry->src_ip.addr.ipv4 ) &&
                 ( connection_entry_dst_ip.addr.ipv4 == xi_lookup_entry->dst_ip.addr.ipv4 ) ) ||
               ( ( ( xi_lookup_entry->dst_ip.family == bdmf_ip_family_ipv6 ) &&
                   ( connection_entry_src_ip.addr.ipv4 == xi_ipv6_src_ip_crc ) &&
                   ( connection_entry_dst_ip.addr.ipv4 == xi_ipv6_dst_ip_crc ) ) ) ) )
        {
            return ( BL_LILAC_RDD_ERROR_LOOKUP_ENTRY_EXISTS );
        }
    }

    *xo_tries = tries;
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_add ( BL_LILAC_RDD_ADD_CONNECTION_DTE  *xi_add_connection,
                                                  rdpa_traffic_dir                 xi_direction )
{
    RDD_CONNECTION_TABLE_DTS          *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS          *connection_entry_ptr;
    RDD_CONTEXT_TABLE_DTS             *context_table_ptr;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS  *context_entry_ptr;
    uint8_t                           entry_bytes[ LILAC_RDD_CONNECTION_ENTRY_SIZE ];
    uint32_t                          crc_init_value, crc_result, hash_index, tries;
    uint32_t                          connection_entry_index;
    uint32_t                          context_entry_index;
    uint32_t                          ipv6_src_ip_crc;
    uint32_t                          ipv6_dst_ip_crc;
#if !defined(FIRMWARE_INIT)
    uint32_t                          *ipv6_buffer_ptr;
#endif
    BL_LILAC_RDD_ERROR_DTE            rdd_error;
    uint32_t                          bucket_overflow_counter;
    uint32_t                          entry_overflow;
    unsigned long                     flags;
    uint32_t                          ip_checksum_delta;
    uint32_t                          l4_checksum_delta;

    if ( xi_add_connection->lookup_entry->dst_ip.family != bdmf_ip_family_ipv4 )
    {
        if ( xi_add_connection->context_entry.actions_vector & rdpa_fc_action_nat )
        {
            return ( BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY );
        }

        if ( xi_direction != rdpa_dir_us )
        {
            if ( xi_add_connection->context_entry.actions_vector & ( rdpa_fc_action_dslite_tunnel | rdpa_fc_action_gre_tunnel ) )
            {
                return ( BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY );
            }
        }
    }

    if ( xi_add_connection->context_entry.egress_port != BL_LILAC_RDD_PCI_BRIDGE_PORT )
    {
        if ( xi_add_connection->context_entry.wifi_ssid != 0 )
        {
            return ( BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY );
        }
    }

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    if ( xi_direction == rdpa_dir_ds )
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    ipv6_src_ip_crc = 0;
    ipv6_dst_ip_crc = 0;

    entry_bytes[ 0 ] = 0;
    entry_bytes[ 1 ] = 0;
    entry_bytes[ 2 ] = 0;
    entry_bytes[ 3 ] = 0;
    entry_bytes[ 4 ] = ( xi_add_connection->lookup_entry->src_port >> 8 ) & 0xFF;
    entry_bytes[ 5 ] = xi_add_connection->lookup_entry->src_port & 0xFF;
    entry_bytes[ 6 ] = ( xi_add_connection->lookup_entry->dst_port >> 8 ) & 0xFF;
    entry_bytes[ 7 ] = xi_add_connection->lookup_entry->dst_port & 0xFF;

    if ( xi_add_connection->lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        entry_bytes[ 8 ] = ( xi_add_connection->lookup_entry->src_ip.addr.ipv4 >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( xi_add_connection->lookup_entry->src_ip.addr.ipv4 >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( xi_add_connection->lookup_entry->src_ip.addr.ipv4 >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = xi_add_connection->lookup_entry->src_ip.addr.ipv4 & 0xFF;
        entry_bytes[ 12 ] = ( xi_add_connection->lookup_entry->dst_ip.addr.ipv4 >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( xi_add_connection->lookup_entry->dst_ip.addr.ipv4 >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( xi_add_connection->lookup_entry->dst_ip.addr.ipv4 >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = xi_add_connection->lookup_entry->dst_ip.addr.ipv4 & 0xFF;
    }
    else
    {
#if !defined(FIRMWARE_INIT)

        ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

        MWRITE_BLK_8( ipv6_buffer_ptr, xi_add_connection->lookup_entry->src_ip.addr.ipv6.data, 16 );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( rdd_error );
        }

        ipv6_src_ip_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
        ipv6_src_ip_crc = xi_add_connection->lookup_entry->src_ip.addr.ipv4;
#endif
        entry_bytes[ 8 ] = ( ipv6_src_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( ipv6_src_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( ipv6_src_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = ( ipv6_src_ip_crc >> 0 ) & 0xFF;

#if !defined(FIRMWARE_INIT)

        MWRITE_BLK_8( ipv6_buffer_ptr, xi_add_connection->lookup_entry->dst_ip.addr.ipv6.data, 16 );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( rdd_error );
        }

        ipv6_dst_ip_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
        ipv6_dst_ip_crc = xi_add_connection->lookup_entry->dst_ip.addr.ipv4;
#endif
        entry_bytes[ 12 ] = ( ipv6_dst_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( ipv6_dst_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( ipv6_dst_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( ipv6_dst_ip_crc >> 0 ) & 0xFF;

#if !defined(FIRMWARE_INIT)
        xi_add_connection->context_entry.nat_ip.addr.ipv4 = *(uint32_t *)&(xi_add_connection->lookup_entry->dst_ip.addr.ipv6.data[ 12 ]);
#endif
    }

    crc_init_value = xi_add_connection->lookup_entry->prot;

    entry_overflow = LILAC_RDD_FALSE;

    /* calculate the CRC on the connection entry */
    crc_result = rdd_crc_bit_by_bit ( &entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    rdd_error = f_rdd_connection_entry_alloc ( hash_index, connection_table_ptr, xi_add_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );


    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        rdd_error = f_rdd_connection_entry_alloc ( hash_index, connection_table_ptr, xi_add_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( rdd_error );
        }

        if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
        }

        entry_overflow = LILAC_RDD_TRUE;

        /*  increment bucket_overflow_counter in the last entry of the previous bucket */
        if ( hash_index == 0 )
        {
            connection_entry_index = RDD_CONNECTION_TABLE_SIZE - 1;
        }
        else
        {
            connection_entry_index = hash_index - 1;
        }

        connection_entry_ptr = &( connection_table_ptr->entry[ connection_entry_index ] );

        RDD_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_READ ( bucket_overflow_counter, connection_entry_ptr );
        bucket_overflow_counter++;
        RDD_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_WRITE ( bucket_overflow_counter, connection_entry_ptr );
    }

    connection_entry_index = hash_index + tries;

    connection_entry_ptr = &( connection_table_ptr->entry[ connection_entry_index ] );

    xi_add_connection->context_entry.ip_version = xi_add_connection->lookup_entry->dst_ip.family;

    xi_add_connection->context_entry.conn_index = connection_entry_index;
    xi_add_connection->context_entry.conn_dir = xi_direction;

    _rdd_calculate_connection_delta(xi_add_connection, xi_direction, &ip_checksum_delta, &l4_checksum_delta);

    rdd_error = _rdd_context_entry_add(&xi_add_connection->context_entry, &context_entry_index, 
        ip_checksum_delta, l4_checksum_delta);

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    if ( xi_add_connection->lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        RDD_CONNECTION_ENTRY_DST_IP_WRITE ( xi_add_connection->lookup_entry->dst_ip.addr.ipv4, connection_entry_ptr );
        RDD_CONNECTION_ENTRY_SRC_IP_WRITE ( xi_add_connection->lookup_entry->src_ip.addr.ipv4, connection_entry_ptr );
    }
    else
    {
        RDD_CONNECTION_ENTRY_DST_IP_WRITE ( ipv6_dst_ip_crc ,connection_entry_ptr );
        RDD_CONNECTION_ENTRY_SRC_IP_WRITE ( ipv6_src_ip_crc ,connection_entry_ptr );
    }
    RDD_CONNECTION_ENTRY_DST_PORT_WRITE ( xi_add_connection->lookup_entry->dst_port, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_PORT_WRITE ( xi_add_connection->lookup_entry->src_port, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_PROTOCOL_WRITE ( xi_add_connection->lookup_entry->prot, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_KEY_EXTEND_WRITE ( xi_add_connection->lookup_entry->dst_ip.family, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_WRITE ( context_entry_index, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_ON, connection_entry_ptr );

    /* return the index of the entry in the table */
    xi_add_connection->xo_entry_index = context_entry_index;

    if ( entry_overflow == LILAC_RDD_TRUE )
    {
        /* set entry_overflow in the context of the entry */
        context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
        context_entry_ptr = &(context_table_ptr->entry[ context_entry_index ] );

        RDD_FLOW_CACHE_CONTEXT_ENTRY_OVERFLOW_WRITE ( LILAC_RDD_TRUE, context_entry_ptr );
    }

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

static inline BL_LILAC_RDD_ERROR_DTE f_rdd_connection_entry_lookup ( uint32_t                  xi_hash_index,
                                                                     RDD_CONNECTION_TABLE_DTS  *xi_connection_table_ptr,
                                                                     rdpa_ip_flow_key_t        *xi_lookup_entry,
                                                                     uint32_t                  xi_ipv6_src_ip_crc,
                                                                     uint32_t                  xi_ipv6_dst_ip_crc,
                                                                     uint32_t                  *xo_tries )
{
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  tries;
    uint32_t                  connection_entry_index;
    uint32_t                  connection_entry_valid;
    uint8_t                   connection_entry_protocol;
    uint16_t                  connection_entry_src_port;
    uint16_t                  connection_entry_dst_port;
    bdmf_ip_t                 connection_entry_src_ip;
    bdmf_ip_t                 connection_entry_dst_ip;
 
    for ( tries = 0; tries < LILAC_RDD_CONNECTION_TABLE_SET_SIZE; tries++ )
    {
        connection_entry_index = xi_hash_index + tries;

        connection_entry_ptr = &( xi_connection_table_ptr->entry[ connection_entry_index ] );

        RDD_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );

        if ( connection_entry_valid )
        {
            RDD_CONNECTION_ENTRY_PROTOCOL_READ ( connection_entry_protocol, connection_entry_ptr );
            RDD_CONNECTION_ENTRY_SRC_PORT_READ ( connection_entry_src_port, connection_entry_ptr );
            RDD_CONNECTION_ENTRY_DST_PORT_READ ( connection_entry_dst_port, connection_entry_ptr );
            RDD_CONNECTION_ENTRY_SRC_IP_READ ( connection_entry_src_ip.addr.ipv4, connection_entry_ptr );
            RDD_CONNECTION_ENTRY_DST_IP_READ ( connection_entry_dst_ip.addr.ipv4, connection_entry_ptr );

            if ( ( connection_entry_protocol == xi_lookup_entry->prot ) &&
                 ( connection_entry_src_port == xi_lookup_entry->src_port ) &&
                 ( connection_entry_dst_port == xi_lookup_entry->dst_port ) &&
                 ( ( ( xi_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 ) &&
                     ( connection_entry_src_ip.addr.ipv4 == xi_lookup_entry->src_ip.addr.ipv4 ) &&
                     ( connection_entry_dst_ip.addr.ipv4 == xi_lookup_entry->dst_ip.addr.ipv4 ) ) ||
                   ( ( ( xi_lookup_entry->dst_ip.family == bdmf_ip_family_ipv6 ) &&
                       ( connection_entry_src_ip.addr.ipv4 == xi_ipv6_src_ip_crc ) &&
                       ( connection_entry_dst_ip.addr.ipv4 == xi_ipv6_dst_ip_crc ) ) ) ) )
            {
                break;
            }
        }
    }

    *xo_tries = tries;

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_delete(uint32_t context_entry_index)
{
    RDD_CONNECTION_TABLE_DTS          *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS          *connection_entry_ptr;
    RDD_CONTEXT_TABLE_DTS             *context_table_ptr;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS  *context_entry_ptr;
    uint32_t                          connection_entry_valid;
    uint32_t                          connection_entry_context_table_index;
    rdpa_traffic_dir                  context_entry_connection_direction;
    uint32_t                          context_entry_connection_table_index;
    uint32_t                          entry_overflow;
    uint32_t                          connection_entry_index;
    uint32_t                          bucket_overflow_counter;
    unsigned long                     flags;

    /*
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry;

    entry = _rdd_context_entry_get(context_entry_index); 
    if (!entry)
        return BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID;
*/

    if (context_entry_index >= RDD_CONTEXT_TABLE_SIZE)
        return BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID;

    context_table_ptr = (RDD_CONTEXT_TABLE_DTS *)ContextTableBase;

    f_rdd_lock_irq(&int_lock_irq, &flags);

    context_entry_ptr = &(context_table_ptr->entry[context_entry_index]);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ(context_entry_connection_direction, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ(context_entry_connection_table_index, context_entry_ptr);

    if (context_entry_connection_direction == rdpa_dir_ds)
        connection_table_ptr = (RDD_CONNECTION_TABLE_DTS *)DsConnectionTableBase;
    else
        connection_table_ptr = (RDD_CONNECTION_TABLE_DTS *)UsConnectionTableBase;

    connection_entry_ptr = &(connection_table_ptr->entry[context_entry_connection_table_index]);

    RDD_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( connection_entry_context_table_index, connection_entry_ptr );

    if (!connection_entry_valid || connection_entry_context_table_index != context_entry_index)
    {
        f_rdd_unlock_irq(&int_lock_irq, flags);
        return BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY;
    }

    RDD_FLOW_CACHE_CONTEXT_ENTRY_OVERFLOW_READ ( entry_overflow, context_entry_ptr );

    if ( entry_overflow == LILAC_RDD_TRUE )
    {
        /* decrement bucket_overflow_counter in the last entry of the previous bucket */
        if ( context_entry_connection_table_index < LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            connection_entry_index = RDD_CONNECTION_TABLE_SIZE - 1;
        }
        else
        {
            connection_entry_index = context_entry_connection_table_index - ( context_entry_connection_table_index % LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) - 1;
        }

        connection_entry_ptr = &( connection_table_ptr->entry[ connection_entry_index ] );

        RDD_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_READ ( bucket_overflow_counter, connection_entry_ptr );
        bucket_overflow_counter--;
        RDD_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_WRITE ( bucket_overflow_counter, connection_entry_ptr );

        connection_entry_ptr = &( connection_table_ptr->entry[ context_entry_connection_table_index ] );
    }

    RDD_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_OFF, connection_entry_ptr );

    f_rdd_free_context_entry ( context_entry_index );

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_search ( BL_LILAC_RDD_ADD_CONNECTION_DTE  *xi_get_connection,
                                                     rdpa_traffic_dir                 xi_direction,
                                                     uint32_t                         *xo_entry_index )
{
    RDD_CONNECTION_TABLE_DTS  *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint8_t                   entry_bytes[ LILAC_RDD_CONNECTION_ENTRY_SIZE ];
    uint32_t                  crc_init_value, crc_result, hash_index, tries;
    uint32_t                  connection_entry_index;
    uint16_t                  connection_entry_context_index;
    uint32_t                  ipv6_src_ip_crc;
    uint32_t                  ipv6_dst_ip_crc;
#if !defined(FIRMWARE_INIT)
    uint32_t                  *ipv6_buffer_ptr;
    BL_LILAC_RDD_ERROR_DTE    rdd_error;
#endif
    unsigned long             flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    if ( xi_direction == rdpa_dir_ds )
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    ipv6_src_ip_crc = 0;
    ipv6_dst_ip_crc = 0;

    entry_bytes[ 0 ] = 0;
    entry_bytes[ 1 ] = 0;
    entry_bytes[ 2 ] = 0;
    entry_bytes[ 3 ] = 0;
    entry_bytes[ 4 ] = ( xi_get_connection->lookup_entry->src_port >> 8 ) & 0xFF;
    entry_bytes[ 5 ] = xi_get_connection->lookup_entry->src_port & 0xFF;
    entry_bytes[ 6 ] = ( xi_get_connection->lookup_entry->dst_port >> 8 ) & 0xFF;
    entry_bytes[ 7 ] = xi_get_connection->lookup_entry->dst_port & 0xFF;

    if ( xi_get_connection->lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        entry_bytes[ 8 ] = ( xi_get_connection->lookup_entry->src_ip.addr.ipv4 >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( xi_get_connection->lookup_entry->src_ip.addr.ipv4 >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( xi_get_connection->lookup_entry->src_ip.addr.ipv4 >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = xi_get_connection->lookup_entry->src_ip.addr.ipv4 & 0xFF;
        entry_bytes[ 12 ] = ( xi_get_connection->lookup_entry->dst_ip.addr.ipv4 >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( xi_get_connection->lookup_entry->dst_ip.addr.ipv4 >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( xi_get_connection->lookup_entry->dst_ip.addr.ipv4 >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = xi_get_connection->lookup_entry->dst_ip.addr.ipv4 & 0xFF;
    }
    else
    {
#if !defined(FIRMWARE_INIT)

        ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

        MWRITE_BLK_8( ipv6_buffer_ptr, xi_get_connection->lookup_entry->src_ip.addr.ipv6.data, 16 );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( rdd_error );
        }

        ipv6_src_ip_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
        ipv6_src_ip_crc = xi_get_connection->lookup_entry->src_ip.addr.ipv4;
#endif
        entry_bytes[ 8 ] = ( ipv6_src_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( ipv6_src_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( ipv6_src_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = ( ipv6_src_ip_crc >> 0 ) & 0xFF;

#if !defined(FIRMWARE_INIT)

        MWRITE_BLK_8( ipv6_buffer_ptr, xi_get_connection->lookup_entry->dst_ip.addr.ipv6.data, 16 );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( rdd_error );
        }

        ipv6_dst_ip_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
        ipv6_dst_ip_crc = xi_get_connection->lookup_entry->dst_ip.addr.ipv4;
#endif
        entry_bytes[ 12 ] = ( ipv6_dst_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( ipv6_dst_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( ipv6_dst_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( ipv6_dst_ip_crc >> 0 ) & 0xFF;
    }

    crc_init_value = xi_get_connection->lookup_entry->prot;

    /* calculate the CRC on the connection entry */
    crc_result = rdd_crc_bit_by_bit ( &entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    f_rdd_connection_entry_lookup ( hash_index, connection_table_ptr, xi_get_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        f_rdd_connection_entry_lookup ( hash_index, connection_table_ptr, xi_get_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );

        if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
        }
    }

    connection_entry_index = hash_index + tries;

    connection_entry_ptr = &( connection_table_ptr->entry[ connection_entry_index ] );

    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( connection_entry_context_index, connection_entry_ptr );
    *xo_entry_index = connection_entry_context_index;

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

static RDD_CONNECTION_ENTRY_DTS *_rdd_connection_entry_get(rdpa_traffic_dir dir, uint32_t conn_entry_index)
{
    RDD_CONNECTION_TABLE_DTS *conn_table;
    RDD_CONNECTION_ENTRY_DTS *conn_entry;
    uint8_t valid;

    if (dir == rdpa_dir_ds)
        conn_table = (RDD_CONNECTION_TABLE_DTS *)DsConnectionTableBase;
    else
        conn_table = (RDD_CONNECTION_TABLE_DTS *)UsConnectionTableBase;

    conn_entry = &(conn_table->entry[conn_entry_index]);
 
    RDD_CONNECTION_ENTRY_VALID_READ(valid, conn_entry);
    if (!valid)
        return NULL;
    return conn_entry;
}

static RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *_rdd_context_entry_get(uint32_t context_entry_index)
{
    RDD_CONNECTION_ENTRY_DTS *conn_entry;
    RDD_CONTEXT_TABLE_DTS *context_table;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *context_entry;
    uint32_t context_index;
    rdpa_traffic_dir dir;
    uint32_t conn_table_index;

    if (context_entry_index >= RDD_CONTEXT_TABLE_SIZE)
        return NULL;

    context_table = (RDD_CONTEXT_TABLE_DTS *)ContextTableBase;
    context_entry = &(context_table->entry[context_entry_index]);
    
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ(dir, context_entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ(conn_table_index, context_entry);

    conn_entry = _rdd_connection_entry_get(dir, conn_table_index);
    if (!conn_entry)
        return NULL;
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ(context_index, conn_entry);
    if (context_entry_index != context_index)
        return NULL;
    return context_entry;
}

BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_get ( rdpa_traffic_dir    xi_direction,
                                                  uint32_t            xi_entry_index,
                                                  rdpa_ip_flow_key_t  *xo_connection_entry,
                                                  uint32_t            *xo_context_index )
{
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;

    connection_entry_ptr = _rdd_connection_entry_get(xi_direction, xi_entry_index);

    RDD_CONNECTION_ENTRY_PROTOCOL_READ ( xo_connection_entry->prot, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_PORT_READ ( xo_connection_entry->src_port, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_PORT_READ ( xo_connection_entry->dst_port, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_IP_READ ( xo_connection_entry->src_ip.addr.ipv4, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_IP_READ ( xo_connection_entry->dst_ip.addr.ipv4, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_KEY_EXTEND_READ ( xo_connection_entry->dst_ip.family ,connection_entry_ptr );
    xo_connection_entry->src_ip.family = xo_connection_entry->dst_ip.family;
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( *xo_context_index ,connection_entry_ptr );

    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_context_entry_get(uint32_t context_entry_index, rdd_fc_context_t *context_entry)
{
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry;

    entry = _rdd_context_entry_get(context_entry_index); 
    if (!entry)
        return BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID;
    /* It's possible that we just need to validate the context entry exist, but not really need the context itself */
    if (context_entry)
        _rdd_fc_context_read_from_table_entry(context_entry, entry);
    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_context_entry_modify(rdd_fc_context_t *context_entry, uint32_t context_entry_index)
{
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry;
    unsigned long flags;
    BL_LILAC_RDD_ERROR_DTE rdd_error;

    entry = _rdd_context_entry_get(context_entry_index); 
    if (!entry)
        return BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID;

    f_rdd_lock_irq(&int_lock_irq, &flags);
    _rdd_fc_context_write_to_table_entry(context_entry, entry, 0);

    rdd_error = f_rdd_cpu_tx_send_message(LILAC_RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY,
        context_entry->conn_dir == rdpa_dir_ds ? FAST_RUNNER_A : FAST_RUNNER_B,
        context_entry->conn_dir == rdpa_dir_ds ? RUNNER_PRIVATE_0_OFFSET : RUNNER_PRIVATE_1_OFFSET,
        context_entry_index, 0, 0, BL_LILAC_RDD_WAIT);

    f_rdd_unlock_irq (&int_lock_irq, flags);
    return rdd_error;
}

BL_LILAC_RDD_ERROR_DTE rdd_connections_number_get ( uint32_t  *xo_connections_number )
{
    *xo_connections_number = RDD_CONTEXT_TABLE_SIZE - g_free_context_entries_number;

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_clear_connection_table ( void )
{
    RDD_CONNECTION_TABLE_DTS  *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint16_t                  entry_index;
    unsigned long             flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;

    for ( entry_index = 0; entry_index < RDD_CONNECTION_TABLE_SIZE; entry_index++ )
    {
        connection_entry_ptr = &( connection_table_ptr->entry[ entry_index ] );

        if ( connection_entry_ptr->valid )
        {
            f_rdd_free_context_entry ( connection_entry_ptr->context_index );
        }

        RDD_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_OFF, connection_entry_ptr );
    }

    connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;

    for ( entry_index = 0; entry_index < RDD_CONNECTION_TABLE_SIZE; entry_index++ )
    {
        connection_entry_ptr = &( connection_table_ptr->entry[ entry_index ] );

        if ( connection_entry_ptr->valid )
        {
            f_rdd_free_context_entry( connection_entry_ptr->context_index );
        }

        RDD_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_OFF, connection_entry_ptr );
    }

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_layer4_filter_set ( RDD_LAYER4_FILTER_INDEX                xi_filter_index,
                                               BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                               uint8_t                                xi_filter_parameter,
                                               rdpa_traffic_dir                       xi_direction )
{
    RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_DTS  *ds_layer4_filter_context_table_ptr;
    RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_DTS  *us_layer4_filter_context_table_ptr;
    RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DTS  *layer4_filter_context_entry_ptr;

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_layer4_filter_context_table_ptr = RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_PTR();
        layer4_filter_context_entry_ptr = &( ds_layer4_filter_context_table_ptr->entry[ xi_filter_index ] );
    }
    else
    {
        us_layer4_filter_context_table_ptr = RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_PTR();
        layer4_filter_context_entry_ptr = &( us_layer4_filter_context_table_ptr->entry[ xi_filter_index ] );
    }

    if ( xi_filter_action == BL_LILAC_RDD_LAYER4_FILTER_DROP )
    {
        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_OFF, layer4_filter_context_entry_ptr );
    }
    else if ( xi_filter_action == BL_LILAC_RDD_LAYER4_FILTER_CPU_TRAP )
    {
        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_ON, layer4_filter_context_entry_ptr );
        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_OFF, layer4_filter_context_entry_ptr );
        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( xi_filter_parameter, layer4_filter_context_entry_ptr );
    }
    else if ( xi_filter_action == BL_LILAC_RDD_LAYER4_FILTER_FORWARD )
    {
        if ( ( xi_filter_index != RDD_LAYER4_FILTER_GRE ) &&
             ( xi_filter_index != RDD_LAYER4_FILTER_ESP ) &&
             ( xi_filter_index != RDD_LAYER4_FILTER_LAYER3_IPV4 ) &&
             ( xi_filter_index != RDD_LAYER4_FILTER_LAYER3_IPV6 ) )
        {
            return ( BL_LILAC_RDD_ERROR_CAM_INSERTION_FAILED );
        }

        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE ( xi_filter_parameter, layer4_filter_context_entry_ptr );
        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_OFF, layer4_filter_context_entry_ptr );
        RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE ( LILAC_RDD_OFF, layer4_filter_context_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_header_error_filter_config ( BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                                        uint8_t                                xi_filter_parameter,
                                                        rdpa_traffic_dir                       xi_direction )
{
    if ( xi_filter_action == BL_LILAC_RDD_LAYER4_FILTER_FORWARD )
    {
        return ( BL_LILAC_RDD_ERROR_CAM_INSERTION_FAILED );
    }

    rdd_layer4_filter_set ( RDD_LAYER4_FILTER_ERROR, xi_filter_action, xi_filter_parameter, xi_direction );
    rdd_layer4_filter_set ( RDD_LAYER4_FILTER_EXCEPTION, xi_filter_action, xi_filter_parameter, xi_direction );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ip_fragments_filter_config( BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                                       uint8_t                                xi_filter_parameter,
                                                       rdpa_traffic_dir                       xi_direction )
{
    if ( xi_filter_action == BL_LILAC_RDD_LAYER4_FILTER_FORWARD )
    {
        return ( BL_LILAC_RDD_ERROR_CAM_INSERTION_FAILED );
    }

    rdd_layer4_filter_set ( RDD_LAYER4_FILTER_IP_FIRST_FRAGMENT, xi_filter_action, xi_filter_parameter, xi_direction );
    rdd_layer4_filter_set ( RDD_LAYER4_FILTER_IP_FRAGMENT, xi_filter_action, xi_filter_parameter, xi_direction );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ds_connection_miss_action_filter_config ( BL_LILAC_RDD_FILTER_MODE_DTE  xi_enable )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_DS_CONNECTION_MISS_ACTION_WRITE ( xi_enable, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_firewall_initialize ( void )
{
    RDD_FIREWALL_RULES_TABLE_DTS                   *firewall_rules_table_ptr;
    RDD_FIREWALL_RULES_MAP_TABLE_DTS               *firewall_rules_map_table_ptr;
    RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_DTS  *firewall_cfg_reg_ptr;

    /* set the entire rule table to NULL rule index */
    firewall_rules_map_table_ptr = ( RDD_FIREWALL_RULES_MAP_TABLE_DTS * )FirewallRulesMapTable;

    MEMSET ( firewall_rules_map_table_ptr, 0, sizeof ( RDD_FIREWALL_RULES_MAP_TABLE_DTS ) );

    /* set the entire context rule table to NULL */
    firewall_rules_table_ptr = ( RDD_FIREWALL_RULES_TABLE_DTS * )FirewallRulesTable;

    MEMSET ( firewall_rules_table_ptr, 0, sizeof ( RDD_FIREWALL_RULES_TABLE_DTS ) );

    /* write firewall configration register */
    firewall_cfg_reg_ptr = ( RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + FIREWALL_CONFIGURATION_REGISTER_ADDRESS );

    RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_RULES_MAP_TABLE_ADDRESS_WRITE ( ( g_runner_tables_ptr + FIREWALL_RULES_MAP_TABLE_ADDRESS ) & 0x1FFFFFFF, firewall_cfg_reg_ptr );
    RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_RULES_TABLE_ADDRESS_WRITE ( ( g_runner_tables_ptr + FIREWALL_RULES_TABLE_ADDRESS ) & 0x1FFFFFFF, firewall_cfg_reg_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_add ( uint32_t                  xi_rule_index,
                                               RDD_FIREWALL_RULE_PARAMS  *xi_firewall_rule_params )
{
    RDD_FIREWALL_RULES_TABLE_DTS      *firewall_rules_table_ptr;
    RDD_FIREWALL_RULE_ENTRY_DTS       *firewall_rule_entry_ptr;
    RDD_FIREWALL_RULES_MAP_TABLE_DTS  *firewall_rules_map_table_ptr;
    RDD_FIREWALL_RULES_MAP_ENTRY_DTS  *firewall_rule_map_entry_ptr;
    uint32_t                          dst_port_index;
    uint32_t                          rule_index;
    uint32_t                          next_rule;
#if !defined(FIRMWARE_INIT)
    BL_LILAC_RDD_ERROR_DTE            rdd_error;
    uint32_t                          *ipv6_buffer_ptr;
#endif
    uint32_t                          ipv6_crc;
    unsigned long                     flags;

    if ( xi_firewall_rule_params->subnet_id > BL_LILAC_RDD_SUBNET_FLOW_CACHE )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_SUBNET_ID );
    }

    /* rule 0 in application means rule 255 in memory */
    if ( xi_rule_index == 0 )
    {
        rule_index = RDD_FIREWALL_RULES_TABLE_SIZE - 1;
    }
    else
    {
        rule_index = xi_rule_index;
    }

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    firewall_rules_map_table_ptr = ( RDD_FIREWALL_RULES_MAP_TABLE_DTS * )FirewallRulesMapTable;

    firewall_rules_table_ptr = ( RDD_FIREWALL_RULES_TABLE_DTS * )FirewallRulesTable;

    /* fetch current rule, if exists, to later chain it as next rule */
    firewall_rule_map_entry_ptr = &( firewall_rules_map_table_ptr->entry[ xi_firewall_rule_params->subnet_id ][ xi_firewall_rule_params->protocol ][ xi_firewall_rule_params->dst_port ] );

    RDD_FIREWALL_RULES_MAP_ENTRY_RULE_INDEX_READ ( next_rule, firewall_rule_map_entry_ptr );

    /* write rule index to rules map table */
    for ( dst_port_index = xi_firewall_rule_params->dst_port; dst_port_index <= xi_firewall_rule_params->dst_port_last; dst_port_index++ )
    {
        firewall_rule_map_entry_ptr = &( firewall_rules_map_table_ptr->entry[ xi_firewall_rule_params->subnet_id ][ xi_firewall_rule_params->protocol ][ dst_port_index ] );

        RDD_FIREWALL_RULES_MAP_ENTRY_RULE_INDEX_WRITE ( rule_index, firewall_rule_map_entry_ptr );
    }

    firewall_rule_entry_ptr = &( firewall_rules_table_ptr->entry[ rule_index ] );

    /* write next rule to rule entry */
    RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_WRITE ( next_rule, firewall_rule_entry_ptr );

    /*  raise flags in rule entry according to input parameters and write them to rule entry */
    if ( xi_firewall_rule_params->src_ip_mask > LILAC_RDD_FIREWALL_RULES_MASK_MAX_LENGTH - 1 )
    {
        RDD_FIREWALL_RULE_ENTRY_CHECK_MASK_SRC_IP_WRITE ( LILAC_RDD_FALSE, firewall_rule_entry_ptr );
        RDD_FIREWALL_RULE_ENTRY_SRC_IP_MASK_WRITE ( 0, firewall_rule_entry_ptr );
    }
    else
    {
        RDD_FIREWALL_RULE_ENTRY_CHECK_MASK_SRC_IP_WRITE ( LILAC_RDD_TRUE, firewall_rule_entry_ptr );
        RDD_FIREWALL_RULE_ENTRY_SRC_IP_MASK_WRITE ( LILAC_RDD_FIREWALL_RULES_MASK_MAX_LENGTH - xi_firewall_rule_params->src_ip_mask, firewall_rule_entry_ptr );
    }

    if ( xi_firewall_rule_params->check_src_ip == LILAC_RDD_FALSE )
    {
        RDD_FIREWALL_RULE_ENTRY_CHECK_SRC_IP_WRITE ( LILAC_RDD_FALSE, firewall_rule_entry_ptr );
        RDD_FIREWALL_RULE_ENTRY_SRC_IP_WRITE ( 0, firewall_rule_entry_ptr );
    }
    else
    {
        RDD_FIREWALL_RULE_ENTRY_CHECK_SRC_IP_WRITE ( LILAC_RDD_TRUE, firewall_rule_entry_ptr );

        if ( xi_firewall_rule_params->src_ip.family == bdmf_ip_family_ipv4 )
        {
            RDD_FIREWALL_RULE_ENTRY_SRC_IP_WRITE ( xi_firewall_rule_params->src_ip.addr.ipv4, firewall_rule_entry_ptr );
        }
        else
        {
#if !defined(FIRMWARE_INIT)

            ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

            MWRITE_BLK_8( ipv6_buffer_ptr, xi_firewall_rule_params->src_ip.addr.ipv6.data, 16 );

            rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                f_rdd_unlock_irq ( &int_lock_irq, flags );
                return ( rdd_error );
            }

            ipv6_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
            ipv6_crc = xi_firewall_rule_params->src_ip.addr.ipv4;
#endif
            RDD_FIREWALL_RULE_ENTRY_SRC_IP_WRITE ( ipv6_crc, firewall_rule_entry_ptr );
        }
    }

    if ( xi_firewall_rule_params->check_dst_ip == LILAC_RDD_FALSE )
    {
        RDD_FIREWALL_RULE_ENTRY_CHECK_DST_IP_WRITE ( LILAC_RDD_FALSE, firewall_rule_entry_ptr );
        RDD_FIREWALL_RULE_ENTRY_DST_IP_WRITE ( 0, firewall_rule_entry_ptr );
    }
    else
    {
        RDD_FIREWALL_RULE_ENTRY_CHECK_DST_IP_WRITE ( LILAC_RDD_TRUE, firewall_rule_entry_ptr );

        if ( xi_firewall_rule_params->dst_ip.family == bdmf_ip_family_ipv4 )
        {
            RDD_FIREWALL_RULE_ENTRY_DST_IP_WRITE ( xi_firewall_rule_params->dst_ip.addr.ipv4, firewall_rule_entry_ptr );
        }
        else
        {
#if !defined(FIRMWARE_INIT)

            ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

            MWRITE_BLK_8( ipv6_buffer_ptr, xi_firewall_rule_params->dst_ip.addr.ipv6.data, 16 );

            rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                f_rdd_unlock_irq ( &int_lock_irq, flags );
                return ( rdd_error );
            }

            ipv6_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
            ipv6_crc = xi_firewall_rule_params->dst_ip.addr.ipv4;
#endif
            RDD_FIREWALL_RULE_ENTRY_DST_IP_WRITE ( ipv6_crc, firewall_rule_entry_ptr );
        }
    }

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_delete ( uint32_t                  xi_rule_index,
                                                  RDD_FIREWALL_RULE_PARAMS  *xi_firewall_rule_params )
{
    RDD_FIREWALL_RULES_MAP_TABLE_DTS  *firewall_rules_map_table_ptr;
    RDD_FIREWALL_RULES_MAP_ENTRY_DTS  *firewall_rule_map_entry_ptr;
    RDD_FIREWALL_RULES_TABLE_DTS      *firewall_rules_table_ptr;
    RDD_FIREWALL_RULE_ENTRY_DTS       *firewall_rule_entry_ptr;
    RDD_FIREWALL_RULE_ENTRY_DTS       *firewall_rule_entry_next_ptr;
    uint32_t                          dst_port_index;
    uint32_t                          rule_index;
    uint32_t                          next_rule;
    unsigned long                     flags;

    if ( xi_firewall_rule_params->subnet_id > BL_LILAC_RDD_SUBNET_FLOW_CACHE )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_SUBNET_ID );
    }

    /* rule 0 in application means rule 255 in memory */
    if ( xi_rule_index == 0 )
    {
        rule_index = RDD_FIREWALL_RULES_TABLE_SIZE - 1;
    }
    else
    {
        rule_index = xi_rule_index;
    }

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    firewall_rules_map_table_ptr = ( RDD_FIREWALL_RULES_MAP_TABLE_DTS * )FirewallRulesMapTable;

    firewall_rules_table_ptr = ( RDD_FIREWALL_RULES_TABLE_DTS * )FirewallRulesTable;

    /* fetch first rule in chain */
    firewall_rule_map_entry_ptr = &( firewall_rules_map_table_ptr->entry[ xi_firewall_rule_params->subnet_id ][ xi_firewall_rule_params->protocol ][ xi_firewall_rule_params->dst_port ] );

    RDD_FIREWALL_RULES_MAP_ENTRY_RULE_INDEX_READ( next_rule, firewall_rule_map_entry_ptr );

    if ( next_rule == rule_index )
    {
        /* if entry is first, change corresponding entries in rules map table to point to next rule (or 0) */
        firewall_rule_entry_ptr = &( firewall_rules_table_ptr->entry[ next_rule ] );

        RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_READ ( next_rule, firewall_rule_entry_ptr );

        /* write rule index to rules map table */
        for ( dst_port_index = xi_firewall_rule_params->dst_port; dst_port_index <= xi_firewall_rule_params->dst_port_last; dst_port_index++ )
        {
            firewall_rule_map_entry_ptr = &( firewall_rules_map_table_ptr->entry[ xi_firewall_rule_params->subnet_id ][ xi_firewall_rule_params->protocol ][ dst_port_index ] );

            RDD_FIREWALL_RULES_MAP_ENTRY_RULE_INDEX_WRITE ( next_rule, firewall_rule_map_entry_ptr );
        }
    }
    else
    {
        /* If entry isn't first, go down rules chain linked list, and delete the correct rule */
        while ( next_rule != 0 )
        {
            firewall_rule_entry_ptr = &( firewall_rules_table_ptr->entry[ next_rule ] );

            RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_READ ( next_rule, firewall_rule_entry_ptr );

            if ( next_rule == rule_index )
            {
                firewall_rule_entry_next_ptr = &( firewall_rules_table_ptr->entry[ next_rule ] );

                RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_READ ( next_rule, firewall_rule_entry_next_ptr );

                /* write next rule to rule entry */
                RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_WRITE ( next_rule, firewall_rule_entry_ptr );

                break;
            }

            RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_READ ( next_rule, firewall_rule_entry_ptr );
        }
    }

    /* annulize entry in rules table */
    firewall_rule_entry_ptr = &( firewall_rules_table_ptr->entry[ rule_index ] );

    MEMSET ( firewall_rule_entry_ptr, 0, sizeof ( RDD_FIREWALL_RULE_ENTRY_DTS ) );

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_search ( uint32_t                  *xo_rule_index,
                                                  RDD_FIREWALL_RULE_PARAMS  *firewall_rule_params )
{
    RDD_FIREWALL_RULES_MAP_TABLE_DTS  *firewall_rules_map_table_ptr;
    RDD_FIREWALL_RULES_MAP_ENTRY_DTS  *firewall_rule_map_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE            rdd_error;
    uint32_t                          rule_index;
    unsigned long                     flags;

    if ( firewall_rule_params->subnet_id > BL_LILAC_RDD_SUBNET_FLOW_CACHE )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_SUBNET_ID );
    }

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    firewall_rules_map_table_ptr = ( RDD_FIREWALL_RULES_MAP_TABLE_DTS * )FirewallRulesMapTable;

    firewall_rule_map_entry_ptr = &( firewall_rules_map_table_ptr->entry[ firewall_rule_params->subnet_id ][ firewall_rule_params->protocol ][ firewall_rule_params->dst_port ] );

    RDD_FIREWALL_RULES_MAP_ENTRY_RULE_INDEX_READ ( rule_index, firewall_rule_map_entry_ptr );

    /* rule 0 in application means rule 255 in memory */
    if ( rule_index == RDD_FIREWALL_RULES_TABLE_SIZE - 1 )
    {
        *xo_rule_index = 0;
    }
    else
    {
        *xo_rule_index = rule_index;
    }

    rdd_error = rdd_firewall_rule_get ( *xo_rule_index, firewall_rule_params );

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_get ( uint32_t                  xi_rule_index,
                                               RDD_FIREWALL_RULE_PARAMS  *xo_firewall_rule_params )
{
    RDD_FIREWALL_RULES_TABLE_DTS  *firewall_rules_table_ptr;
    RDD_FIREWALL_RULE_ENTRY_DTS   *firewall_rule_entry_ptr;
    uint32_t                      src_ip_mask;

    /* rule 0 in application means rule 255 in memory */
    if ( xi_rule_index == 0 )
    {
        xi_rule_index = RDD_FIREWALL_RULES_TABLE_SIZE - 1;
    }

    firewall_rules_table_ptr = ( RDD_FIREWALL_RULES_TABLE_DTS * )FirewallRulesTable;

    firewall_rule_entry_ptr = &( firewall_rules_table_ptr->entry[ xi_rule_index ] );

    RDD_FIREWALL_RULE_ENTRY_CHECK_DST_IP_READ ( xo_firewall_rule_params->check_dst_ip, firewall_rule_entry_ptr );
    RDD_FIREWALL_RULE_ENTRY_CHECK_SRC_IP_READ ( xo_firewall_rule_params->check_src_ip, firewall_rule_entry_ptr );
    RDD_FIREWALL_RULE_ENTRY_CHECK_MASK_SRC_IP_READ ( xo_firewall_rule_params->check_mask_src_ip, firewall_rule_entry_ptr );
    RDD_FIREWALL_RULE_ENTRY_SRC_IP_MASK_READ ( src_ip_mask, firewall_rule_entry_ptr );
    xo_firewall_rule_params->src_ip_mask = ( LILAC_RDD_FIREWALL_RULES_MASK_MAX_LENGTH - src_ip_mask ) % LILAC_RDD_FIREWALL_RULES_MASK_MAX_LENGTH;
    RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_READ ( xo_firewall_rule_params->next_rule_index, firewall_rule_entry_ptr );
    RDD_FIREWALL_RULE_ENTRY_DST_IP_READ ( xo_firewall_rule_params->dst_ip.addr.ipv4, firewall_rule_entry_ptr );
    RDD_FIREWALL_RULE_ENTRY_SRC_IP_READ ( xo_firewall_rule_params->src_ip.addr.ipv4, firewall_rule_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ipv6_ecn_remark ( uint32_t  xi_control )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ECN_REMARK_WRITE ( xi_control, bridge_cfg_register );

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ECN_REMARK_WRITE ( xi_control, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_3_tupples_connection_mode_config ( bdmf_boolean  xi_3_tupple_mode )
{
    RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_DTS  *ds_layer4_filter_table_ptr;
    RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_DTS  *us_layer4_filter_table_ptr;
    RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DTS  *layer4_filter_entry_ptr;

    ds_layer4_filter_table_ptr = RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_PTR();
    us_layer4_filter_table_ptr = RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_PTR();

    if ( xi_3_tupple_mode )
    {
        rdd_layer4_filter_set ( RDD_LAYER4_FILTER_LAYER3_IPV4, BL_LILAC_RDD_LAYER4_FILTER_FORWARD, 0, rdpa_dir_ds);
        rdd_layer4_filter_set ( RDD_LAYER4_FILTER_LAYER3_IPV4, BL_LILAC_RDD_LAYER4_FILTER_FORWARD, 0, rdpa_dir_us);
        
        rdd_layer4_filter_set ( RDD_LAYER4_FILTER_LAYER3_IPV6, BL_LILAC_RDD_LAYER4_FILTER_FORWARD, 0, rdpa_dir_ds);
        rdd_layer4_filter_set ( RDD_LAYER4_FILTER_LAYER3_IPV6, BL_LILAC_RDD_LAYER4_FILTER_FORWARD, 0, rdpa_dir_us);


        layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0x0, layer4_filter_entry_ptr );

        layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0x0, layer4_filter_entry_ptr );

        layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0x0, layer4_filter_entry_ptr );

        layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0x0, layer4_filter_entry_ptr );
    }
    else
    {
        layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );

        layer4_filter_entry_ptr = &( ds_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );

        layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV4 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );

        layer4_filter_entry_ptr = &( us_layer4_filter_table_ptr->entry[ RDD_LAYER4_FILTER_LAYER3_IPV6 ] );
        RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( 0xC, layer4_filter_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


void rdd_full_flow_cache_acceleration_config ( rdd_full_fc_acceleration_mode  xi_mode,
                                               rdpa_traffic_dir               xi_direction,
                                               bdmf_boolean                   xi_enable )
{
    if ( xi_mode == rdd_full_fc_acceleration_non_ip )
    {
        f_global_ingress_vector_config ( GLOBAL_INGRESS_CONFIG_NON_IP_FC_ACCELRATION, xi_direction, xi_enable );
    }
    else
    {
        f_global_ingress_vector_config ( GLOBAL_INGRESS_CONFIG_IP_MULTICAST_FC_ACCELERATION, xi_direction, xi_enable );
    }
}


void f_rdd_full_flow_cache_config ( bdmf_boolean  xi_control )
{
    f_global_ingress_vector_config ( GLOBAL_INGRESS_CONFIG_FULL_FLOW_CACHE_MODE, rdpa_dir_ds, xi_control );
}


void f_global_ingress_vector_config ( uint32_t          xi_filter_id,
                                      rdpa_traffic_dir  xi_direction,
                                      bdmf_boolean      xi_control )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    uint8_t                                global_ingress_config;

    if ( xi_direction == rdpa_dir_ds )
    {
        bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    }
    else
    {
        bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    }

    RDD_BRIDGE_CONFIGURATION_REGISTER_GLOBAL_INGRESS_CONFIG_READ ( global_ingress_config, bridge_cfg_register );

    if ( xi_control )
    {
        global_ingress_config |= ( BL_LILAC_RDD_FILTER_ENABLE << xi_filter_id );
    }
    else
    {
        global_ingress_config &= ~( BL_LILAC_RDD_FILTER_ENABLE << xi_filter_id );
    }

    RDD_BRIDGE_CONFIGURATION_REGISTER_GLOBAL_INGRESS_CONFIG_WRITE ( global_ingress_config, bridge_cfg_register );

    return;
}
