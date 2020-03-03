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
#if defined(FIRMWARE_INIT)
#include <strings.h>
#endif

#define RDD_FC_MCAST_CONNECTION2_NEXT_INVALID   (RDD_FC_MCAST_CONNECTION2_TABLE_SIZE-1)

/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

#if defined(FIRMWARE_INIT)
extern uint8_t *DsConnectionTableBase;
extern uint8_t *UsConnectionTableBase;
#endif

extern uint8_t *ContextTableBase;
extern uint8_t *g_runner_ddr_base_addr;
extern uint32_t g_runner_ddr_base_addr_phys;
extern uint8_t *g_runner_tables_ptr;
extern uint32_t  g_free_context_entries_number;
extern uint32_t  g_free_context_entries_head;
extern uint32_t  g_free_context_entries_tail;
extern uint32_t  *g_free_connection_context_entries;
extern RDD_64_BIT_TABLE_CFG  g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
extern RDD_DDR_TABLE_CFG     g_ddr_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];

extern bdmf_fastlock int_lock_irq;


static BL_LILAC_RDD_ERROR_DTE f_rdd_free_context_entry ( uint32_t  context_entry_index )
{
    RDD_CONTEXT_TABLE_DTS *context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
    RDD_CONTEXT_ENTRY_UNION_DTS *context_entry_ptr = &(context_table_ptr->entry[ context_entry_index ] );

    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_VALID_WRITE( 0, context_entry_ptr );
    g_free_connection_context_entries [ g_free_context_entries_tail++ ] = context_entry_index;
    g_free_context_entries_tail = g_free_context_entries_tail % RDD_CONTEXT_TABLE_SIZE;
    g_free_context_entries_number++;

    return ( BL_LILAC_RDD_OK );
}

int rdd_fc_global_cfg_get(RDD_FC_GLOBAL_CFG_ENTRY_DTS *xo_global_cfg)
{
    uint8_t *global_cfg_config_ptr;

    global_cfg_config_ptr = (uint8_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FC_GLOBAL_CFG_ADDRESS);
    RDD_FC_GLOBAL_CFG_ENTRY_FC_ACCEL_MODE_READ(xo_global_cfg->fc_accel_mode, global_cfg_config_ptr);
    RDD_FC_GLOBAL_CFG_ENTRY_FC_TCP_ACK_MFLOWS_READ(xo_global_cfg->fc_tcp_ack_mflows, global_cfg_config_ptr);

    return BDMF_ERR_OK;
}

int rdd_fc_global_cfg_set(RDD_FC_GLOBAL_CFG_ENTRY_DTS *xi_global_cfg)
{
    uint8_t *global_cfg_config_ptr;

    global_cfg_config_ptr = (uint8_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FC_GLOBAL_CFG_ADDRESS);
    RDD_FC_GLOBAL_CFG_ENTRY_FC_ACCEL_MODE_WRITE(xi_global_cfg->fc_accel_mode, global_cfg_config_ptr);
    RDD_FC_GLOBAL_CFG_ENTRY_FC_TCP_ACK_MFLOWS_WRITE(xi_global_cfg->fc_tcp_ack_mflows, global_cfg_config_ptr);

    global_cfg_config_ptr = (uint8_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FC_GLOBAL_CFG_ADDRESS);
    RDD_FC_GLOBAL_CFG_ENTRY_FC_ACCEL_MODE_WRITE(xi_global_cfg->fc_accel_mode, global_cfg_config_ptr);
    RDD_FC_GLOBAL_CFG_ENTRY_FC_TCP_ACK_MFLOWS_WRITE(xi_global_cfg->fc_tcp_ack_mflows, global_cfg_config_ptr);

    return BDMF_ERR_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_free_context_entry ( uint32_t  context_entry_index )
{
    return f_rdd_free_context_entry ( context_entry_index );
}

BL_LILAC_RDD_ERROR_DTE f_rdd_connection_table_initialize ( void )
{
    uint32_t  *connection_table_config_ptr;
    uint32_t  connection_table_address;
    uint32_t  *context_table_config_ptr;
    uint32_t  context_table_address;
    uint32_t  i;
    RUNNER_REGS_CFG_CAM_CFG   runner_cam_configuration_register;

    runner_cam_configuration_register.stop_value = 0xFFFF;
    RUNNER_REGS_0_CFG_CAM_CFG_WRITE ( runner_cam_configuration_register );
    RUNNER_REGS_1_CFG_CAM_CFG_WRITE ( runner_cam_configuration_register );

    connection_table_address = RDD_RSV_VIRT_TO_PHYS( g_runner_ddr_base_addr, g_runner_ddr_base_addr_phys,
       g_runner_tables_ptr + DS_CONNECTION_TABLE_ADDRESS );
    connection_table_config_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CONNECTION_TABLE_CONFIG_ADDRESS );
    MWRITE_32( connection_table_config_ptr,  connection_table_address );

    connection_table_address = RDD_RSV_VIRT_TO_PHYS( g_runner_ddr_base_addr, g_runner_ddr_base_addr_phys,
        g_runner_tables_ptr + US_CONNECTION_TABLE_ADDRESS );
    connection_table_config_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CONNECTION_TABLE_CONFIG_ADDRESS );
    MWRITE_32( connection_table_config_ptr,  connection_table_address );

    context_table_address = RDD_RSV_VIRT_TO_PHYS( g_runner_ddr_base_addr, g_runner_ddr_base_addr_phys,
       g_runner_tables_ptr + CONTEXT_TABLE_ADDRESS );
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


static BL_LILAC_RDD_ERROR_DTE f_rdd_context_entry_write ( RDD_CONTEXT_ENTRY_UNION_DTS  *xi_context_entry,
                                                          RDD_CONTEXT_ENTRY_UNION_DTS  *xi_context_entry_ptr,
                                                          uint32_t is_new_entry )
{
    uint32_t i;

    if( xi_context_entry->fc_ucast_flow_context_entry.multicast_flag == 0 )
    {
        if(is_new_entry)
        {
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_WRITE( 0, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_WRITE( 0, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_WRITE( 0, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_WRITE ( 0, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_ROUTED_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_routed, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_L2_ACCEL_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_l2_accel, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_DF_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_df, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_MAPT_US_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_mapt_us, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_EGRESS_PHY_WRITE( xi_context_entry->fc_ucast_flow_context_entry.egress_phy, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_ANY_WRITE(xi_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DROP_WRITE(xi_context_entry->fc_ucast_flow_context_entry.drop, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_PRIORITY_WRITE(xi_context_entry->fc_ucast_flow_context_entry.priority, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_WFD_PRIO_WRITE(xi_context_entry->fc_ucast_flow_context_entry.wfd_prio, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_WFD_IDX_WRITE(xi_context_entry->fc_ucast_flow_context_entry.wfd_idx, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_NIC_WRITE(xi_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_HIT_TRAP_WRITE(xi_context_entry->fc_ucast_flow_context_entry.is_hit_trap, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CPU_REASON_WRITE( xi_context_entry->fc_ucast_flow_context_entry.cpu_reason, xi_context_entry_ptr );

            if (xi_context_entry->fc_ucast_flow_context_entry.egress_phy == rdd_egress_phy_wlan)
            {
                if (xi_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any)
                {
                    if (xi_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic)
                    {
                        RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_CHAIN_IDX_WRITE(xi_context_entry->fc_ucast_flow_context_wfd_nic_entry.chain_idx, xi_context_entry_ptr);
                    }
                    else
                    {
                        RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_WIFI_SSID_WRITE(xi_context_entry->fc_ucast_flow_context_wfd_dhd_entry.wifi_ssid, xi_context_entry_ptr);
                        RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_FLOW_RING_ID_WRITE(xi_context_entry->fc_ucast_flow_context_wfd_dhd_entry.flow_ring_id, xi_context_entry_ptr);
                    }
                }
#if defined(CONFIG_DHD_RUNNER)
                else
                {
                    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_RADIO_IDX_WRITE(xi_context_entry->fc_ucast_flow_context_rnr_dhd_entry.radio_idx, xi_context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_WIFI_SSID_WRITE(xi_context_entry->fc_ucast_flow_context_rnr_dhd_entry.wifi_ssid, xi_context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_FLOW_RING_ID_WRITE(xi_context_entry->fc_ucast_flow_context_rnr_dhd_entry.flow_ring_id, xi_context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DHD_FLOW_PRIORITY_WRITE(xi_context_entry->fc_ucast_flow_context_rnr_dhd_entry.dhd_flow_priority, xi_context_entry_ptr);
                }
#endif
            }
            else
            {
                RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_EGRESS_MODE_WRITE(xi_context_entry->fc_ucast_flow_context_eth_xtm_entry.egress_info, xi_context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_EGRESS_PORT_WRITE(xi_context_entry->fc_ucast_flow_context_eth_xtm_entry.egress_port, xi_context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_TRAFFIC_CLASS_WRITE(xi_context_entry->fc_ucast_flow_context_eth_xtm_entry.traffic_class, xi_context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_RATE_CONTROLLER_WRITE(xi_context_entry->fc_ucast_flow_context_eth_xtm_entry.rate_controller, xi_context_entry_ptr);
            }
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_DIRECTION_WRITE ( xi_context_entry->fc_ucast_flow_context_entry.connection_direction, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_WRITE ( xi_context_entry->fc_ucast_flow_context_entry.connection_table_index, xi_context_entry_ptr );

            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_LENGTH_64_WRITE ( xi_context_entry->fc_ucast_flow_context_entry.command_list_length_64, xi_context_entry_ptr );

            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IP_ADDRESSES_TABLE_INDEX_WRITE( xi_context_entry->fc_ucast_flow_context_entry.ip_addresses_table_index, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_PATHSTAT_IDX_WRITE( xi_context_entry->fc_ucast_flow_context_entry.pathstat_idx, xi_context_entry_ptr );

            for ( i = 0; i < RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER; i++ )
            {
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_WRITE ( xi_context_entry->fc_ucast_flow_context_entry.command_list[ i ], xi_context_entry_ptr, i );
            }

            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MTU_WRITE( xi_context_entry->fc_ucast_flow_context_entry.mtu, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_tos_mangle, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_TOS_WRITE( xi_context_entry->fc_ucast_flow_context_entry.tos, xi_context_entry_ptr );
            if (xi_context_entry->fc_ucast_flow_context_entry.egress_phy != rdd_egress_phy_wlan)
            {
                RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_TRAFFIC_CLASS_WRITE(xi_context_entry->fc_ucast_flow_context_eth_xtm_entry.traffic_class, xi_context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_RATE_CONTROLLER_WRITE(xi_context_entry->fc_ucast_flow_context_eth_xtm_entry.rate_controller, xi_context_entry_ptr);
            }
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_VALID_WRITE( xi_context_entry->fc_ucast_flow_context_entry.valid, xi_context_entry_ptr );
        }

        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_SERVICE_QUEUE_ID_WRITE( xi_context_entry->fc_ucast_flow_context_entry.service_queue_id, xi_context_entry_ptr );
        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_WRED_HIGH_PRIO_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_wred_high_prio, xi_context_entry_ptr );
        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_INGQOS_HIGH_PRIO_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_ingqos_high_prio, xi_context_entry_ptr );
    }
    else
    {
        uint32_t port_context = 0;
        RDD_FC_MCAST_PORT_CONTEXT_ENTRY_DTS *port_context_ptr;
        uint8_t port_mask = 0;

        if(is_new_entry)
        {
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_WRITE( 0, xi_context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_WRITE( 0, xi_context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_WRITE( 1, xi_context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_WRITE( 0, xi_context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_IS_ROUTED_WRITE( xi_context_entry->fc_mcast_flow_context_entry.is_routed, xi_context_entry_ptr );

#if !defined(BDMF_SYSTEM_SIM)
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_MCAST_PORT_HEADER_BUFFER_PTR_WRITE( xi_context_entry->fc_mcast_flow_context_entry.mcast_port_header_buffer_ptr, xi_context_entry_ptr );
#else
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_MCAST_PORT_HEADER_BUFFER_PTR_WRITE( g_runner_ddr_base_addr + SIMULATOR_DDR_PORT_HEADER_BUFFERS_OFFSET, xi_context_entry_ptr );
#endif
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_DIRECTION_WRITE( xi_context_entry->fc_mcast_flow_context_entry.connection_direction, xi_context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_WRITE( xi_context_entry->fc_mcast_flow_context_entry.connection_table_index, xi_context_entry_ptr );

            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_LENGTH_64_WRITE ( xi_context_entry->fc_mcast_flow_context_entry.command_list_length_64, xi_context_entry_ptr );

            for ( i = 0; i < RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER; i++ )
            {
                RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_WRITE ( xi_context_entry->fc_mcast_flow_context_entry.l3_command_list[ i ], xi_context_entry_ptr, i );
            }
        }

        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_WLAN_MCAST_CLIENTS_WRITE( xi_context_entry->fc_mcast_flow_context_entry.wlan_mcast_clients,
                                                                  xi_context_entry_ptr );
        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE( xi_context_entry->fc_mcast_flow_context_entry.wlan_mcast_index,
                                                                xi_context_entry_ptr );

        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_MTU_WRITE( xi_context_entry->fc_mcast_flow_context_entry.mtu, xi_context_entry_ptr );
        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_WRITE( xi_context_entry->fc_mcast_flow_context_entry.is_tos_mangle, xi_context_entry_ptr );
        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_TOS_WRITE( xi_context_entry->fc_mcast_flow_context_entry.tos, xi_context_entry_ptr );
        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_VALID_WRITE( xi_context_entry->fc_mcast_flow_context_entry.valid, xi_context_entry_ptr );

        if(is_new_entry)
        {
            port_mask = 0;
        }
        else
        {
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_MASK_READ( port_mask, xi_context_entry_ptr );
        }

        for ( i = 0; i < RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_NUMBER; i++ )
        {
            uint16_t number_ports_port_mask = ((uint16_t) xi_context_entry->fc_mcast_flow_context_entry.port_mask) |
                ((uint16_t) xi_context_entry->fc_mcast_flow_context_entry.number_of_ports << 8);

            uint8_t old_port_mask = port_mask & (1 << i);
            uint8_t new_port_mask = xi_context_entry->fc_mcast_flow_context_entry.port_mask & (1 << i);

            if(old_port_mask ^ new_port_mask)
            {
                if(old_port_mask)
                {
                    /* Remove Port */

                    /* Remove WLAN MCAST egress port from port mask */
                    number_ports_port_mask &= ~(1 << WLAN_MCAST_EGRESS_PORT_INDEX);

                    /* Write out number_of_ports and port_mask fields in one operation. */
                    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_NUM_PORTS_PORT_MASK_WRITE( number_ports_port_mask, xi_context_entry_ptr );
                }
                else
                {
                    /* Add Port */

                    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_READ ( port_context, xi_context_entry_ptr, i );
                    port_context = swap4bytes(port_context);

                    port_context_ptr = (RDD_FC_MCAST_PORT_CONTEXT_ENTRY_DTS *) &xi_context_entry->fc_mcast_flow_context_entry.port_context[ i ];

                    RDD_FC_MCAST_PORT_CONTEXT_ENTRY_STATE_WRITE(port_context_ptr->state, &port_context);
                    RDD_FC_MCAST_PORT_CONTEXT_ENTRY_QUEUE_WRITE(port_context_ptr->queue, &port_context);
                    RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_COMMAND_LIST_LENGTH_WRITE(port_context_ptr->l2_command_list_length, &port_context);
                    RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_HEADER_LENGTH_WRITE(port_context_ptr->l2_header_length, &port_context);
                    RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_PUSH_WRITE(port_context_ptr->l2_push, &port_context);
                    RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_OFFSET_WRITE(port_context_ptr->l2_offset, &port_context);
                    RDD_FC_MCAST_PORT_CONTEXT_ENTRY_IS_WRED_HIGH_PRIO_WRITE(port_context_ptr->is_wred_high_prio, &port_context);

                    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_WRITE ( swap4bytes(port_context), xi_context_entry_ptr, i );

                    /* Remove WLAN MCAST egress port from port mask */
                    number_ports_port_mask &= ~(1 << WLAN_MCAST_EGRESS_PORT_INDEX);

                    /* Write out number_of_ports and port_mask fields in one operation. */
                    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_NUM_PORTS_PORT_MASK_WRITE( number_ports_port_mask, xi_context_entry_ptr );
                }

#if !defined(FIRMWARE_INIT)
                /* Only one port context chage is supported at a time */
                break;
#endif
            }
        }
    }

    return ( BL_LILAC_RDD_OK );
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_context_entry_add ( RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry,
                                                        rdpa_traffic_dir             xi_direction,
                                                        uint32_t                     *context_entry_index )
{
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;

    if ( g_free_context_entries_number > LILAC_RDD_RESERVED_CONTEXT_ENTRIES )
    {
        *context_entry_index = g_free_connection_context_entries[ g_free_context_entries_head++ ];
        g_free_context_entries_head = g_free_context_entries_head % RDD_CONTEXT_TABLE_SIZE;
        g_free_context_entries_number--;
    }
    else
    {
        return ( BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY );
    }

    context_entry->fc_mcast_flow_context_entry.valid = 1;
    context_entry_ptr = &( context_table_ptr->entry[ *context_entry_index ] );

    f_rdd_context_entry_write ( context_entry, context_entry_ptr, 1 );

#if defined(CC_RDD_ROUTER_DEBUG)
    {
        uint32_t context_entry_connection_table_index;

        if (context_entry_ptr->fc_ucast_flow_context_entry.multicast_flag == 0)
        {
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ ( context_entry_connection_table_index, context_entry_ptr );

            __debug("%s, %u: connection_table_index %u, context_entry_index %u, context_entry_ptr %p\n", __FUNCTION__, __LINE__, 
                context_entry_connection_table_index, *context_entry_index, context_entry_ptr);
        }
        else
        {
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_DTS *mcast_context_entry_ptr = (RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_DTS *) context_entry_ptr;

            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ ( context_entry_connection_table_index, mcast_context_entry_ptr );

            __debug("%s, %u: connection_table_index %u, context_entry_index %u, context_entry_ptr %p\n", __FUNCTION__, __LINE__, 
                context_entry_connection_table_index, *context_entry_index, context_entry_ptr);
        }
    }
#endif

    return ( BL_LILAC_RDD_OK );
}


/* f_rdd_ipproto_lookup_port_get() gets IP Proto (TCP, UDP, etc.) and source bridge port from the protocol fields */
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_ipproto_lookup_port_get ( uint8_t    *xi_prot_ptr,
                                                                     uint8_t    *xo_lookup_port_ptr,
                                                                     uint8_t    *xo_lookup_tcp_pure_ack )
{
    uint8_t ipproto_idx;
    uint8_t ipproto;

    *xo_lookup_tcp_pure_ack = 0;
    *xo_lookup_port_ptr = (*xi_prot_ptr >> TUPLE_PROTO_LOOKUP_PORT_F_OFFSET) & TUPLE_PROTO_LOOKUP_PORT_F_MASK;
    ipproto_idx = (*xi_prot_ptr >> TUPLE_PROTO_PROTOCOL_F_OFFSET) & TUPLE_PROTO_PROTOCOL_F_MASK;

    switch (ipproto_idx)
    {
        case IPPROTO_IDX_TCP_ACK:
            *xo_lookup_tcp_pure_ack = 1;
            /* fall through */

        case IPPROTO_IDX_TCP:
            ipproto = IPPROTO_TCP;
            break;

        case IPPROTO_IDX_UDP:
            ipproto = IPPROTO_UDP;
            break;

        case IPPROTO_IDX_IPIP:
            ipproto = IPPROTO_IPIP;
            break;

        case IPPROTO_IDX_IPV6:
            ipproto = IPPROTO_IPV6;
            break;

        case IPPROTO_IDX_GRE:
            ipproto = IPPROTO_GRE;
            break;

        default:
            ipproto = IPPROTO_UDP;
    }

    *xi_prot_ptr = ipproto;

    return ( BL_LILAC_RDD_OK );
}


/* f_rdd_prot_set() sets protocol field using IP Proto (TCP, UDP, etc.) and connection port */
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_prot_set ( uint8_t    *xi_prot_ptr,
                                                      uint8_t    xi_lookup_port,
                                                      uint8_t    xi_tcp_pure_ack )
{
    uint8_t ipproto_idx;

    switch (*xi_prot_ptr)
    {
        case IPPROTO_TCP:
            ipproto_idx = (xi_tcp_pure_ack) ? IPPROTO_IDX_TCP_ACK : IPPROTO_IDX_TCP;
            break;

        case IPPROTO_UDP:
            ipproto_idx = IPPROTO_IDX_UDP;
            break;

        case IPPROTO_IPIP:
            ipproto_idx = IPPROTO_IDX_IPIP;
            break;

        case IPPROTO_IPV6:
            ipproto_idx = IPPROTO_IDX_IPV6;
            break;

        case IPPROTO_GRE:
            ipproto_idx = IPPROTO_IDX_GRE;
            break;

        default:
            ipproto_idx = IPPROTO_IDX_UNDEF;
    }

    *xi_prot_ptr = (ipproto_idx << TUPLE_PROTO_PROTOCOL_F_OFFSET) | (xi_lookup_port & TUPLE_PROTO_LOOKUP_PORT_F_MASK);

    return ( BL_LILAC_RDD_OK );
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
    uint8_t                   connection_entry_lookup_port;
    uint8_t                   connection_entry_tcp_pure_ack;
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

        f_rdd_ipproto_lookup_port_get ( &connection_entry_protocol, &connection_entry_lookup_port,
            &connection_entry_tcp_pure_ack );

        if ( ( connection_entry_protocol == xi_lookup_entry->prot ) &&
             ( connection_entry_tcp_pure_ack == xi_lookup_entry->tcp_pure_ack ) &&
             ( connection_entry_lookup_port == xi_lookup_entry->lookup_port ) &&
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
    RDD_CONNECTION_TABLE_DTS     *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS     *connection_entry_ptr;
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint8_t                      entry_bytes[ LILAC_RDD_CONNECTION_ENTRY_SIZE ];
    uint32_t                     crc_init_value, crc_result, hash_index, tries;
    uint32_t                     connection_entry_index;
    uint32_t                     context_entry_index;
    uint32_t                     ipv6_src_ip_crc;
    uint32_t                     ipv6_dst_ip_crc;
    BL_LILAC_RDD_ERROR_DTE       rdd_error;
    uint32_t                     bucket_overflow_counter;
    uint32_t                     entry_overflow;
    unsigned long                flags;
    uint8_t                      connection_entry_protocol;
    uint16_t                     any_src_port_flow_counter;
    RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS *any_src_port_flow_counter_ptr;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    connection_entry_protocol = xi_add_connection->lookup_entry->prot;
    f_rdd_prot_set ( &connection_entry_protocol, xi_add_connection->lookup_entry->lookup_port,
        xi_add_connection->lookup_entry->tcp_pure_ack );

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
        ipv6_src_ip_crc = crcbitbybit((uint8_t *)&xi_add_connection->lookup_entry->src_ip.addr.ipv6.data,
                                             16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        entry_bytes[ 8 ] = ( ipv6_src_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( ipv6_src_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( ipv6_src_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = ( ipv6_src_ip_crc >> 0 ) & 0xFF;

        ipv6_dst_ip_crc = crcbitbybit((uint8_t *)&xi_add_connection->lookup_entry->dst_ip.addr.ipv6.data,
                                             16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        entry_bytes[ 12 ] = ( ipv6_dst_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( ipv6_dst_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( ipv6_dst_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( ipv6_dst_ip_crc >> 0 ) & 0xFF;
    }

    crc_init_value = connection_entry_protocol;

    entry_overflow = LILAC_RDD_FALSE;

    /* calculate the CRC on the connection entry */
    crc_result = crcbitbybit ( &entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    rdd_error = f_rdd_connection_entry_alloc ( hash_index, connection_table_ptr, xi_add_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );


    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        rdd_error = f_rdd_connection_entry_alloc ( hash_index, connection_table_ptr, xi_add_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
            return ( rdd_error );
        }

        if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_NO_EMPTY_ENTRY );
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


    if( xi_add_connection->context_entry.fc_ucast_flow_context_entry.multicast_flag == 0 )
    {
        xi_add_connection->context_entry.fc_ucast_flow_context_entry.connection_table_index = connection_entry_index;
        xi_add_connection->context_entry.fc_ucast_flow_context_entry.connection_direction = xi_direction;
    }

#if defined(CC_RDD_ROUTER_DEBUG)
    rdd_flow_dump(&xi_add_connection->context_entry, xi_direction);
#endif

    rdd_error = f_rdd_context_entry_add ( &xi_add_connection->context_entry, xi_direction, &context_entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    if (xi_add_connection->lookup_entry->lookup_port == BL_LILAC_RDD_ANY_BRIDGE_PORT)
    { 
        if ( xi_direction == rdpa_dir_ds )
        {
            any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 
        }
        else
        {
            any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 
        }

        MREAD_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
        any_src_port_flow_counter++;
        MWRITE_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
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
    RDD_CONNECTION_ENTRY_PROTOCOL_WRITE ( connection_entry_protocol, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_COMMAND_LIST_LENGTH_64_WRITE( xi_add_connection->context_entry.fc_ucast_flow_context_entry.command_list_length_64, connection_entry_ptr );
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

        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_WRITE ( LILAC_RDD_TRUE, context_entry_ptr );
    }

#if defined(CC_RDD_ROUTER_DEBUG)
    {
        context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
        context_entry_ptr = &(context_table_ptr->entry[ context_entry_index ] );

        __debug("%s, %u: connection_table_index %u\n", __FUNCTION__, __LINE__, context_entry_ptr->fc_ucast_flow_context_entry.connection_table_index);
    }
#endif

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
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
    uint8_t                   connection_entry_lookup_port;
    uint8_t                   connection_entry_tcp_pure_ack;
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

            f_rdd_ipproto_lookup_port_get ( &connection_entry_protocol, &connection_entry_lookup_port,
                &connection_entry_tcp_pure_ack );

            if ( ( connection_entry_protocol == xi_lookup_entry->prot ) &&
                 ( connection_entry_tcp_pure_ack == xi_lookup_entry->tcp_pure_ack ) &&
                 ( connection_entry_lookup_port == xi_lookup_entry->lookup_port ) &&
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


BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_delete ( bdmf_index  xi_entry_index )
{
    RDD_CONNECTION_TABLE_DTS     *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS     *connection_entry_ptr;
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint32_t                     connection_entry_valid;
    uint32_t                     connection_entry_context_table_index;
    uint8_t                      connection_entry_protocol;
    uint8_t                      connection_entry_lookup_port;
    uint8_t                      connection_entry_tcp_pure_ack;
    rdpa_traffic_dir             context_entry_connection_direction;
    uint32_t                     context_entry_connection_table_index;
    uint32_t                     entry_overflow;
    uint32_t                     connection_entry_index;
    uint32_t                     bucket_overflow_counter;
    unsigned long                flags;
    uint16_t                     any_src_port_flow_counter;
    RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS *any_src_port_flow_counter_ptr;

    if ( xi_entry_index >= RDD_CONTEXT_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
    }

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    context_entry_ptr = &( context_table_ptr->entry[ xi_entry_index ] );

    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ ( context_entry_connection_direction, context_entry_ptr );
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ ( context_entry_connection_table_index, context_entry_ptr );

    if ( context_entry_connection_direction == rdpa_dir_ds )
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    connection_entry_ptr = &( connection_table_ptr->entry[ context_entry_connection_table_index ] );

    RDD_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( connection_entry_context_table_index, connection_entry_ptr );

    if ( connection_entry_valid && ( connection_entry_context_table_index == xi_entry_index ) )
    {
        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_READ ( entry_overflow, context_entry_ptr );

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

        RDD_CONNECTION_ENTRY_PROTOCOL_READ ( connection_entry_protocol, connection_entry_ptr );
        RDD_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_OFF, connection_entry_ptr );

        f_rdd_ipproto_lookup_port_get ( &connection_entry_protocol, &connection_entry_lookup_port,
            &connection_entry_tcp_pure_ack );

        if (connection_entry_lookup_port == BL_LILAC_RDD_ANY_BRIDGE_PORT)
        {
            if ( context_entry_connection_direction == rdpa_dir_ds )
            {
                any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 
            }
            else
            {
                any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 
            }

            MREAD_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
            any_src_port_flow_counter--;
            MWRITE_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
        }

#if !defined(FIRMWARE_INIT)
        wmb();
#endif

        /* Delete connection cache from cam_lkp table */
        f_rdd_cpu_tx_send_message( LILAC_RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY,
                               ( context_entry_connection_direction == rdpa_dir_ds) ? FAST_RUNNER_A : FAST_RUNNER_B,
                               ( context_entry_connection_direction == rdpa_dir_ds) ? RUNNER_PRIVATE_0_OFFSET : RUNNER_PRIVATE_1_OFFSET,
                               xi_entry_index, 0, 0, BL_LILAC_RDD_WAIT );

        f_rdd_free_context_entry ( xi_entry_index );

        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_OK );
    }
    else
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }
}


BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_search ( BL_LILAC_RDD_ADD_CONNECTION_DTE  *xi_get_connection,
                                                     rdpa_traffic_dir                 xi_direction,
                                                     bdmf_index                       *xo_entry_index )
{
    RDD_CONNECTION_TABLE_DTS  *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint8_t                   entry_bytes[ LILAC_RDD_CONNECTION_ENTRY_SIZE ];
    uint32_t                  crc_init_value, crc_result, hash_index, tries;
    uint32_t                  connection_entry_index;
    uint16_t                  connection_entry_context_index;
    uint32_t                  ipv6_src_ip_crc;
    uint32_t                  ipv6_dst_ip_crc;
    unsigned long             flags;
    uint8_t                   connection_entry_protocol;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    connection_entry_protocol = xi_get_connection->lookup_entry->prot;
    f_rdd_prot_set ( &connection_entry_protocol, xi_get_connection->lookup_entry->lookup_port,
        xi_get_connection->lookup_entry->tcp_pure_ack );

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
        ipv6_src_ip_crc = crcbitbybit((uint8_t *)&xi_get_connection->lookup_entry->src_ip.addr.ipv6.data,
                                             16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        entry_bytes[ 8 ] = ( ipv6_src_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( ipv6_src_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( ipv6_src_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = ( ipv6_src_ip_crc >> 0 ) & 0xFF;

        ipv6_dst_ip_crc = crcbitbybit((uint8_t *)&xi_get_connection->lookup_entry->dst_ip.addr.ipv6.data,
                                             16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        entry_bytes[ 12 ] = ( ipv6_dst_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( ipv6_dst_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( ipv6_dst_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( ipv6_dst_ip_crc >> 0 ) & 0xFF;
    }

    crc_init_value = connection_entry_protocol;

    /* calculate the CRC on the connection entry */
    crc_result = crcbitbybit ( &entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    f_rdd_connection_entry_lookup ( hash_index, connection_table_ptr, xi_get_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        f_rdd_connection_entry_lookup ( hash_index, connection_table_ptr, xi_get_connection->lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );

        if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
        }
    }

    connection_entry_index = hash_index + tries;

    connection_entry_ptr = &( connection_table_ptr->entry[ connection_entry_index ] );

    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( connection_entry_context_index, connection_entry_ptr );
    *xo_entry_index = connection_entry_context_index;

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_get ( rdpa_traffic_dir    xi_direction,
                                                  uint32_t            xi_entry_index,
                                                  rdpa_ip_flow_key_t  *xo_connection_entry,
                                                  bdmf_index          *xo_context_index )
{
    RDD_CONNECTION_TABLE_DTS  *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  connection_entry_valid;
    uint32_t                  connection_entry_is_multicast;

    if ( xi_direction == rdpa_dir_ds )
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * ) DsConnectionTableBase;
    }
    else
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * ) UsConnectionTableBase;
    }

    connection_entry_ptr = &( connection_table_ptr->entry[ xi_entry_index ] );

    RDD_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );

    if ( !connection_entry_valid )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_FC_MCAST_CONNECTION_ENTRY_IS_MULTICAST_READ ( connection_entry_is_multicast, connection_entry_ptr );

    if ( connection_entry_is_multicast )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_CONNECTION_ENTRY_PROTOCOL_READ ( xo_connection_entry->prot, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_PORT_READ ( xo_connection_entry->src_port, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_PORT_READ ( xo_connection_entry->dst_port, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_IP_READ ( xo_connection_entry->src_ip.addr.ipv4, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_IP_READ ( xo_connection_entry->dst_ip.addr.ipv4, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_KEY_EXTEND_READ ( xo_connection_entry->dst_ip.family ,connection_entry_ptr );
    xo_connection_entry->src_ip.family = xo_connection_entry->dst_ip.family;
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( *xo_context_index ,connection_entry_ptr );

    f_rdd_ipproto_lookup_port_get ( &xo_connection_entry->prot, &xo_connection_entry->lookup_port,
        &xo_connection_entry->tcp_pure_ack );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_context_entry_get ( bdmf_index                   xi_entry_index,
                                               RDD_CONTEXT_ENTRY_UNION_DTS  *xo_context_entry )
{
    RDD_CONNECTION_TABLE_DTS     *connection_table_ptr;
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint32_t                     valid;
    uint32_t                     multicast_flag;
    uint32_t                     connection_direction;

    if ( xi_entry_index >= RDD_CONTEXT_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
    }

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;

    context_entry_ptr = &(context_table_ptr->entry[ xi_entry_index ] );

    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_VALID_READ(valid, context_entry_ptr ); 

    if(!valid)
    {
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
    }

    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_READ( multicast_flag, context_entry_ptr );

    if( !multicast_flag )
    {
        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ ( connection_direction, context_entry_ptr );
    }
    else
    {
        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ( connection_direction, context_entry_ptr );
    }

    if( connection_direction == rdpa_dir_ds )
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    if( !multicast_flag )
    {
        RDD_CONNECTION_ENTRY_DTS *connection_entry_ptr;
        uint32_t connection_table_index;
        uint32_t context_index;
        uint32_t i;

        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ ( connection_table_index, context_entry_ptr );

        connection_entry_ptr = &( connection_table_ptr->entry[ connection_table_index ] );

        RDD_CONNECTION_ENTRY_VALID_READ ( valid, connection_entry_ptr );

        if ( valid )
        {
            RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( context_index, connection_entry_ptr );

            if ( xi_entry_index == context_index )
            {
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_READ( xo_context_entry->fc_ucast_flow_context_entry.flow_hits, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_READ( xo_context_entry->fc_ucast_flow_context_entry.flow_bytes, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_READ( xo_context_entry->fc_ucast_flow_context_entry.multicast_flag, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_READ ( xo_context_entry->fc_ucast_flow_context_entry.overflow, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_ROUTED_READ( xo_context_entry->fc_ucast_flow_context_entry.is_routed, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_L2_ACCEL_READ( xo_context_entry->fc_ucast_flow_context_entry.is_l2_accel, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_WRED_HIGH_PRIO_READ( xo_context_entry->fc_ucast_flow_context_entry.is_wred_high_prio, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_INGQOS_HIGH_PRIO_READ( xo_context_entry->fc_ucast_flow_context_entry.is_ingqos_high_prio, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_DF_READ( xo_context_entry->fc_ucast_flow_context_entry.is_df, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_SERVICE_QUEUE_ID_READ( xo_context_entry->fc_ucast_flow_context_entry.service_queue_id, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_MAPT_US_READ( xo_context_entry->fc_ucast_flow_context_entry.is_mapt_us, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_EGRESS_PHY_READ( xo_context_entry->fc_ucast_flow_context_entry.egress_phy, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MTU_READ( xo_context_entry->fc_ucast_flow_context_entry.mtu, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_READ( xo_context_entry->fc_ucast_flow_context_entry.is_tos_mangle, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_TOS_READ( xo_context_entry->fc_ucast_flow_context_entry.tos, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_ANY_READ(xo_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any, context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DROP_READ(xo_context_entry->fc_ucast_flow_context_entry.drop, context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_PRIORITY_READ(xo_context_entry->fc_ucast_flow_context_entry.priority, context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_WFD_PRIO_READ(xo_context_entry->fc_ucast_flow_context_entry.wfd_prio, context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_WFD_IDX_READ(xo_context_entry->fc_ucast_flow_context_entry.wfd_idx, context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_NIC_READ(xo_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic, context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_PATHSTAT_IDX_READ( xo_context_entry->fc_ucast_flow_context_entry.pathstat_idx, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_HIT_TRAP_READ( xo_context_entry->fc_ucast_flow_context_entry.is_hit_trap, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CPU_REASON_READ( xo_context_entry->fc_ucast_flow_context_entry.cpu_reason, context_entry_ptr );

                if (xo_context_entry->fc_ucast_flow_context_entry.egress_phy == rdd_egress_phy_wlan)
                {
                    if (xo_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any)
                    {
                        if (xo_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic)
                        {
                            RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_CHAIN_IDX_READ(xo_context_entry->fc_ucast_flow_context_wfd_nic_entry.chain_idx, context_entry_ptr);
                        }
                        else
                        {
                            RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_WIFI_SSID_READ(xo_context_entry->fc_ucast_flow_context_wfd_dhd_entry.wifi_ssid, context_entry_ptr);
                            RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_FLOW_RING_ID_READ(xo_context_entry->fc_ucast_flow_context_wfd_dhd_entry.flow_ring_id, context_entry_ptr);
                        }
                    }
#if defined(CONFIG_DHD_RUNNER)
                    else
                    {
                        RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_RADIO_IDX_READ(xo_context_entry->fc_ucast_flow_context_rnr_dhd_entry.radio_idx, context_entry_ptr);
                        RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_WIFI_SSID_READ(xo_context_entry->fc_ucast_flow_context_rnr_dhd_entry.wifi_ssid, context_entry_ptr);
                        RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_FLOW_RING_ID_READ(xo_context_entry->fc_ucast_flow_context_rnr_dhd_entry.flow_ring_id, context_entry_ptr);
                        RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DHD_FLOW_PRIORITY_READ(xo_context_entry->fc_ucast_flow_context_rnr_dhd_entry.dhd_flow_priority, context_entry_ptr);
                    }
#endif
                }
                else
                {
                    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_EGRESS_MODE_READ(xo_context_entry->fc_ucast_flow_context_eth_xtm_entry.egress_info, context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_EGRESS_PORT_READ(xo_context_entry->fc_ucast_flow_context_eth_xtm_entry.egress_port, context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_TRAFFIC_CLASS_READ(xo_context_entry->fc_ucast_flow_context_eth_xtm_entry.traffic_class, context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_RATE_CONTROLLER_READ(xo_context_entry->fc_ucast_flow_context_eth_xtm_entry.rate_controller, context_entry_ptr);
                }

                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ ( xo_context_entry->fc_ucast_flow_context_entry.connection_direction, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ ( xo_context_entry->fc_ucast_flow_context_entry.connection_table_index, context_entry_ptr );

                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_LENGTH_64_READ ( xo_context_entry->fc_ucast_flow_context_entry.command_list_length_64, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_VALID_READ( xo_context_entry->fc_ucast_flow_context_entry.valid, context_entry_ptr );

                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IP_ADDRESSES_TABLE_INDEX_READ( xo_context_entry->fc_ucast_flow_context_entry.ip_addresses_table_index, context_entry_ptr );

                for ( i = 0; i < RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER; i++ )
                {
                    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_READ ( xo_context_entry->fc_ucast_flow_context_entry.command_list[ i ], context_entry_ptr, i );
                }
            }
            else
            {
                return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
            }
        }
        else
        {
            return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
        }
    }
    else
    {
        RDD_CONNECTION_ENTRY_DTS *connection_entry_ptr;
        uint32_t connection_table_index;
        uint32_t i;

        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ( connection_table_index, context_entry_ptr );

        connection_entry_ptr = &( connection_table_ptr->entry[ connection_table_index ] );

        RDD_CONNECTION_ENTRY_VALID_READ ( valid, connection_entry_ptr );

        if ( valid )
        {
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_READ( xo_context_entry->fc_mcast_flow_context_entry.flow_hits, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_READ( xo_context_entry->fc_mcast_flow_context_entry.flow_bytes, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_READ( xo_context_entry->fc_mcast_flow_context_entry.multicast_flag, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_READ( xo_context_entry->fc_mcast_flow_context_entry.overflow, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_IS_ROUTED_READ( xo_context_entry->fc_mcast_flow_context_entry.is_routed, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_MTU_READ( xo_context_entry->fc_mcast_flow_context_entry.mtu, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_READ( xo_context_entry->fc_mcast_flow_context_entry.is_tos_mangle, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_TOS_READ( xo_context_entry->fc_mcast_flow_context_entry.tos, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_WLAN_MCAST_CLIENTS_READ( xo_context_entry->fc_mcast_flow_context_entry.wlan_mcast_clients,
                                                                     context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_WLAN_MCAST_INDEX_READ( xo_context_entry->fc_mcast_flow_context_entry.wlan_mcast_index,
                                                                   context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_MASK_READ( xo_context_entry->fc_mcast_flow_context_entry.port_mask, context_entry_ptr );

            if (xo_context_entry->fc_mcast_flow_context_entry.wlan_mcast_clients)
            {
                /* Add WLAN MCAST egress port to port mask */
                xo_context_entry->fc_mcast_flow_context_entry.port_mask |= (1 << WLAN_MCAST_EGRESS_PORT_INDEX);
            }

            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_NUMBER_OF_PORTS_READ( xo_context_entry->fc_mcast_flow_context_entry.number_of_ports, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_MCAST_PORT_HEADER_BUFFER_PTR_READ( xo_context_entry->fc_mcast_flow_context_entry.mcast_port_header_buffer_ptr, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ( xo_context_entry->fc_mcast_flow_context_entry.connection_direction, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ( xo_context_entry->fc_mcast_flow_context_entry.connection_table_index, context_entry_ptr );
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_VALID_READ(xo_context_entry->fc_mcast_flow_context_entry.valid, context_entry_ptr ); 

            for ( i = 0; i < RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_NUMBER; i++ )
            {
                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_DTS *port_context_ptr;
                uint32_t port_context;

                RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_READ ( port_context, context_entry_ptr, i );
                port_context = swap4bytes(port_context);

                port_context_ptr = (RDD_FC_MCAST_PORT_CONTEXT_ENTRY_DTS *) &xo_context_entry->fc_mcast_flow_context_entry.port_context[ i ];

                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_STATE_READ(port_context_ptr->state, &port_context);
                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_QUEUE_READ(port_context_ptr->queue, &port_context);
                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_COMMAND_LIST_LENGTH_READ(port_context_ptr->l2_command_list_length, &port_context);
                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_HEADER_LENGTH_READ(port_context_ptr->l2_header_length, &port_context);
                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_PUSH_READ(port_context_ptr->l2_push, &port_context);
                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_L2_OFFSET_READ(port_context_ptr->l2_offset, &port_context);
                RDD_FC_MCAST_PORT_CONTEXT_ENTRY_IS_WRED_HIGH_PRIO_READ(port_context_ptr->is_wred_high_prio, &port_context);
            }

            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_LENGTH_64_READ ( xo_context_entry->fc_mcast_flow_context_entry.command_list_length_64, context_entry_ptr );

            for ( i = 0; i < RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER; i++ )
            {
                RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_READ ( xo_context_entry->fc_mcast_flow_context_entry.l3_command_list[ i ], context_entry_ptr, i );
            }
        }
        else
        {
            return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
        }
    }
    return ( BL_LILAC_RDD_OK );
}

/* Get only flwstats related attributes to improve uncached read performance */
BL_LILAC_RDD_ERROR_DTE rdd_context_entry_flwstat_get ( uint32_t                     xi_entry_index,
                                                       RDD_CONTEXT_ENTRY_UNION_DTS  *xo_context_entry )
{
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint32_t                     valid;

    if ( xi_entry_index >= RDD_CONTEXT_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
    }

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;

    context_entry_ptr = &(context_table_ptr->entry[ xi_entry_index ] );

    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_VALID_READ(valid, context_entry_ptr ); 

    if(!valid)
    {
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
    }

    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_READ( xo_context_entry->fc_ucast_flow_context_entry.multicast_flag, context_entry_ptr );
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_L2_ACCEL_READ( xo_context_entry->fc_ucast_flow_context_entry.is_l2_accel, context_entry_ptr );

    /* Multicast and Unicast flow entries have the flow hits and bytes stored in the same offset
       Just use the unicast context entry to store the flow hits and bytes */
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_READ( xo_context_entry->fc_ucast_flow_context_entry.flow_hits, context_entry_ptr );
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_READ( xo_context_entry->fc_ucast_flow_context_entry.flow_bytes, context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_context_entry_modify ( RDD_CONTEXT_ENTRY_UNION_DTS  *xi_context_entry,
                                                  bdmf_index                   xi_entry_index )
{
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    unsigned long                flags;

    if ( xi_entry_index >= RDD_CONTEXT_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
    }

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
    context_entry_ptr = &context_table_ptr->entry[ xi_entry_index ];

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    f_rdd_context_entry_write ( xi_context_entry, context_entry_ptr, 0 );

    f_rdd_cpu_tx_send_message( LILAC_RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY,
                               (xi_context_entry->fc_ucast_flow_context_entry.connection_direction == rdpa_dir_ds) ?
                               FAST_RUNNER_A : FAST_RUNNER_B,
                               (xi_context_entry->fc_ucast_flow_context_entry.connection_direction == rdpa_dir_ds) ?
                               RUNNER_PRIVATE_0_OFFSET : RUNNER_PRIVATE_1_OFFSET,
                               xi_entry_index, 0, 0, BL_LILAC_RDD_WAIT );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );

    return ( BL_LILAC_RDD_OK );
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

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

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

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_3_tupples_connection_mode_config ( bdmf_boolean  xi_3_tupple_mode )
{
    return ( BL_LILAC_RDD_OK );
}

void rdd_full_flow_cache_acceleration_config ( rdd_full_fc_acceleration_mode  xi_mode,
                                               rdpa_traffic_dir               xi_direction,
                                               bdmf_boolean                   xi_enable )
{
}

void f_rdd_full_flow_cache_config ( bdmf_boolean  xi_control )
{
}

BL_LILAC_RDD_ERROR_DTE rdd_ipv6_ecn_remark ( uint32_t  xi_control )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_add ( RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *xi_ip_addresses_entry,
                                                      bdmf_index                          *xo_entry_index,
                                                      uint16_t                            *xo_entry_sram_address )
{
    RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS *fc_flow_ip_addresses_table_ptr;
    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS *entry, curr, available;
    uint8_t                          *addr;
    uint32_t                         i, j;
    BL_LILAC_RDD_ERROR_DTE           rdd_error = BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY;

    *xo_entry_index = *xo_entry_sram_address = xi_ip_addresses_entry->reference_count = 0;

    fc_flow_ip_addresses_table_ptr = ( RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ));

    for( i = 0; i < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE; i++ )
    {
        entry = fc_flow_ip_addresses_table_ptr->entry + i;

        for ( j = 0, addr = curr.sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++ )
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_READ(*addr, entry, j);

        if( !memcmp(curr.sa_da_addresses, xi_ip_addresses_entry->sa_da_addresses, sizeof(curr.sa_da_addresses)) )
        {
            /* Entry is already in the table, update the reference count and return the entry index and address. */
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_READ( xi_ip_addresses_entry->reference_count, entry);
            xi_ip_addresses_entry->reference_count++;
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_WRITE( xi_ip_addresses_entry->reference_count, entry);

            *xo_entry_index = i;
            *xo_entry_sram_address = FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS + (i * sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));
            rdd_error = BL_LILAC_RDD_OK;
            break;
        }
    }

    if( rdd_error != BL_LILAC_RDD_OK )
    {
        memset((uint8_t *) &available, 0x00, sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));

        for( i = 0; i < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE; i++ )
        {
            entry = fc_flow_ip_addresses_table_ptr->entry + i;

            for ( j = 0, addr = curr.sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++ )
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_READ(*addr, entry, j);

            if( !memcmp(curr.sa_da_addresses, available.sa_da_addresses, sizeof(curr.sa_da_addresses)) )
            {
                /* Add new entry */
                for ( j = 0, addr = xi_ip_addresses_entry->sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++ )
                {
                    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_WRITE(*addr, entry, j);
                }
                xi_ip_addresses_entry->reference_count = 1;
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_WRITE( xi_ip_addresses_entry->reference_count, entry);
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_IS_IPV6_ADDRESS_WRITE( xi_ip_addresses_entry->is_ipv6_address, entry );
                *xo_entry_index = i;
                *xo_entry_sram_address = FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS + (i * sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));
                rdd_error = BL_LILAC_RDD_OK;
                break;
            }
        }
    }

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_get ( bdmf_index                          xi_entry_index,
                                                      RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *xo_ip_addresses_entry,
                                                      uint16_t                            *xo_entry_sram_address )
{
    RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS *fc_flow_ip_addresses_table_ptr;
    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS *entry;
    uint8_t                          *addr;
    uint32_t                         j;
    BL_LILAC_RDD_ERROR_DTE           rdd_error = BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY;

    *xo_entry_sram_address = 0;
    memset( (uint8_t *) xo_ip_addresses_entry, 0x00, sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));

    fc_flow_ip_addresses_table_ptr = ( RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ));

    if( xi_entry_index < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE )
    {
        entry = fc_flow_ip_addresses_table_ptr->entry + xi_entry_index;

        RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_READ( xo_ip_addresses_entry->reference_count, entry);
        RDD_FC_FLOW_IP_ADDRESSES_ENTRY_IS_IPV6_ADDRESS_READ( xo_ip_addresses_entry->is_ipv6_address, entry );

        if( xo_ip_addresses_entry->reference_count )
        {
            for ( j = 0, addr = xo_ip_addresses_entry->sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++ )
            {
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_READ(*addr, entry, j);
            }

            *xo_entry_sram_address = FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS + (xi_entry_index * sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));

            rdd_error = BL_LILAC_RDD_OK;
        }
    }

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_delete_by_index ( bdmf_index xi_entry_index )
{
    RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS *fc_flow_ip_addresses_table_ptr;
    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS *entry;
    uint16_t                           reference_count;
    BL_LILAC_RDD_ERROR_DTE             rdd_error = BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY;

    fc_flow_ip_addresses_table_ptr = ( RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ));

    if( xi_entry_index < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE )
    {
        entry = fc_flow_ip_addresses_table_ptr->entry + xi_entry_index;

        RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_READ( reference_count, entry);
        if( reference_count > 0 )
        {
            reference_count--;
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_WRITE( reference_count, entry);
        }

        if(reference_count == 0)
        {
            memset((uint8_t *) entry, 0x00, sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));
        }

        rdd_error = BL_LILAC_RDD_OK;
    }

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_delete_by_address ( RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *xi_ip_addresses_entry )
{
    RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS *fc_flow_ip_addresses_table_ptr;
    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS *entry, curr;
    uint16_t                           reference_count;
    uint32_t                           i, j;
    uint8_t                            *addr;
    BL_LILAC_RDD_ERROR_DTE             rdd_error = BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY;

    fc_flow_ip_addresses_table_ptr = ( RDD_FC_FLOW_IP_ADDRESSES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + FC_FLOW_IP_ADDRESSES_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ));

    for( i = 0; i < RDD_FC_FLOW_IP_ADDRESSES_TABLE_SIZE; i++ )
    {
        entry = fc_flow_ip_addresses_table_ptr->entry + i;

        for ( j = 0, addr = curr.sa_da_addresses; j < RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_NUMBER; j++, addr++ )
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_SA_DA_ADDRESSES_READ(*addr, entry, j);

        if( !memcmp(curr.sa_da_addresses, xi_ip_addresses_entry->sa_da_addresses, sizeof(curr.sa_da_addresses)) )
        {
            RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_READ( reference_count, entry);
            if( reference_count > 0 )
            {
                reference_count--;
                RDD_FC_FLOW_IP_ADDRESSES_ENTRY_REFERENCE_COUNT_WRITE( reference_count, entry);
            }

            if(reference_count == 0)
            {
                memset((uint8_t *) entry, 0x00, sizeof(RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS));
            }

            rdd_error = BL_LILAC_RDD_OK;
            break;
        }
    }

    return ( rdd_error );
}

static inline uint32_t f_number_to_bit_mask_32( uint32_t number )
{
    return ( 1 << number );
}

BL_LILAC_RDD_ERROR_DTE rdd_ucast_ds_wan_udp_filter_get( bdmf_index                       xi_entry_index,
                                                        RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  *xo_ds_wan_udp_filter_entry )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error = BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY;

    if( xi_entry_index < RDD_DS_WAN_UDP_FILTER_TABLE_SIZE )
    {
        RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_DTS *ds_wan_udp_filter_control = ( RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_WAN_UDP_FILTER_CONTROL_TABLE_ADDRESS );
        uint32_t valid_mask;

        RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_VALID_MASK_READ( valid_mask, ds_wan_udp_filter_control );

        if( f_number_to_bit_mask_32( xi_entry_index ) & valid_mask )
        {
            RDD_DS_WAN_UDP_FILTER_ENTRY_DTS *ds_wan_udp_filter_table = ( RDD_DS_WAN_UDP_FILTER_ENTRY_DTS * )RDD_DS_WAN_UDP_FILTER_TABLE_PTR();

            ds_wan_udp_filter_table += xi_entry_index;

            RDD_DS_WAN_UDP_FILTER_ENTRY_OFFSET_READ( xo_ds_wan_udp_filter_entry->offset, ds_wan_udp_filter_table );
            RDD_DS_WAN_UDP_FILTER_ENTRY_VALUE_READ( xo_ds_wan_udp_filter_entry->value, ds_wan_udp_filter_table );
            RDD_DS_WAN_UDP_FILTER_ENTRY_MASK_READ( xo_ds_wan_udp_filter_entry->mask, ds_wan_udp_filter_table );
            RDD_DS_WAN_UDP_FILTER_ENTRY_HITS_READ( xo_ds_wan_udp_filter_entry->hits, ds_wan_udp_filter_table );

            rdd_error = BL_LILAC_RDD_OK;
        }
    }

    return ( rdd_error );
}

#if defined(FIRMWARE_INIT)
static inline int ffz(int mask)
{
    int bit = ffs(~mask);

    if(bit)
    {
        return bit - 1;
    }
    else
    {
        return -1;
    }
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_ucast_ds_wan_udp_filter_add( RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  *xi_ds_wan_udp_filter_entry,
                                                        bdmf_index                       *xo_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error = BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY;
    RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_DTS *ds_wan_udp_filter_control = ( RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_WAN_UDP_FILTER_CONTROL_TABLE_ADDRESS );
    uint32_t valid_mask;
    int entry_index;

    RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_VALID_MASK_READ( valid_mask, ds_wan_udp_filter_control );

    entry_index = ffz( valid_mask );

    if(entry_index >= 0)
    {
        RDD_DS_WAN_UDP_FILTER_ENTRY_DTS *ds_wan_udp_filter_table = ( RDD_DS_WAN_UDP_FILTER_ENTRY_DTS * )RDD_DS_WAN_UDP_FILTER_TABLE_PTR();

        ds_wan_udp_filter_table += entry_index;

        RDD_DS_WAN_UDP_FILTER_ENTRY_OFFSET_WRITE( xi_ds_wan_udp_filter_entry->offset, ds_wan_udp_filter_table );
        RDD_DS_WAN_UDP_FILTER_ENTRY_VALUE_WRITE( xi_ds_wan_udp_filter_entry->value & xi_ds_wan_udp_filter_entry->mask, ds_wan_udp_filter_table );
        RDD_DS_WAN_UDP_FILTER_ENTRY_MASK_WRITE( xi_ds_wan_udp_filter_entry->mask, ds_wan_udp_filter_table );
        RDD_DS_WAN_UDP_FILTER_ENTRY_HITS_WRITE( 0, ds_wan_udp_filter_table );

        valid_mask |= f_number_to_bit_mask_32( entry_index );

        RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_VALID_MASK_WRITE( valid_mask, ds_wan_udp_filter_control );

        *xo_entry_index = (uint32_t)entry_index;

        rdd_error = BL_LILAC_RDD_OK;
    }

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_ucast_ds_wan_udp_filter_delete( bdmf_index xi_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error = BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY;

    if( xi_entry_index < RDD_DS_WAN_UDP_FILTER_TABLE_SIZE )
    {
        RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_DTS *ds_wan_udp_filter_control = ( RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_WAN_UDP_FILTER_CONTROL_TABLE_ADDRESS );
        uint32_t entry_mask = f_number_to_bit_mask_32( xi_entry_index );
        uint32_t valid_mask;

        RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_VALID_MASK_READ( valid_mask, ds_wan_udp_filter_control );

        if( valid_mask & entry_mask )
        {
            valid_mask &= ~entry_mask;

            RDD_DS_WAN_UDP_FILTER_CONTROL_ENTRY_VALID_MASK_WRITE( valid_mask, ds_wan_udp_filter_control );

            rdd_error = BL_LILAC_RDD_OK;
        }
    }

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_port_header_buffer_get( uint32_t xi_index,
                                                            RDD_FC_MCAST_PORT_HEADER_BUFFER_DTS *xi_port_header_entry,
                                                            uint8_t *xo_port_header_buffer )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error = BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID;
    int i;

    if( xi_index < RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE )
    {
        for( i = 0; i < RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE2; i++ )
            RDD_FC_MCAST_PORT_HEADER_ENTRY_U8_READ( xo_port_header_buffer[i], &xi_port_header_entry->entry[xi_index][i] );

        rdd_error = BL_LILAC_RDD_OK;
    }

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_port_header_buffer_put( uint32_t xi_index,
                                                            uint8_t *xi_port_header_buffer,
                                                            RDD_FC_MCAST_PORT_HEADER_BUFFER_DTS *xo_port_header_entry )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error = BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID;
    int i;

    if( xi_index < RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE )
    {
        for( i = 0; i < RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE2; i++ )
            RDD_FC_MCAST_PORT_HEADER_ENTRY_U8_WRITE( xi_port_header_buffer[i], &xo_port_header_entry->entry[xi_index][i] );

        rdd_error = BL_LILAC_RDD_OK;
    }

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_ds_connection_miss_action_filter_config ( BL_LILAC_RDD_FILTER_MODE_DTE  xi_enable )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_ipv6_config ( BL_LILAC_RDD_IPV6_ENABLE_DTE  xi_ipv6_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_subnet_classify_config ( BL_LILAC_RDD_SUBNET_CLASSIFY_MODE_DTE  xi_subnet_classify_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_header_error_filter_config ( BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                                        uint8_t                                xi_filter_parameter,
                                                        rdpa_traffic_dir                       xi_direction )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_layer4_filter_set ( RDD_LAYER4_FILTER_INDEX                xi_filter_index,
                                               BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                               uint8_t                                xi_filter_parameter,
                                               rdpa_traffic_dir                       xi_direction )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_ip_fragments_filter_config( BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                                       uint8_t                                xi_filter_parameter,
                                                       rdpa_traffic_dir                       xi_direction )
{
    return ( BL_LILAC_RDD_OK );
}

void rdd_dual_stack_lite_enable ( bdmf_boolean  xi_ds_lite_enable )
{
}


#if defined(CC_RDD_ROUTER_DEBUG)
extern int g_dbg_lvl;
#define __debug_mcast(fmt, arg...)                          \
    if ( g_dbg_lvl > 0 )                                    \
        do {                                                \
            print(fmt, ##arg);                              \
        } while(0)

#else
#define __debug_mcast(fmt, arg...)
#endif

extern RDD_CONNECTION_TABLE_DTS            *g_ds_connection_table_ptr;
extern RDD_FC_MCAST_CONNECTION2_TABLE_DTS  *g_fc_mcast_connection2_table_ptr;

/* Finds out whether the IP address (IPv4/6) is zero or non-zero.
   Returns: 0 if IP address is zero,
            1 if IP address is non-zero
           -1 if IP address in not IPv4/6
*/
static inline int32_t is_ip_addr_non_zero( bdmf_ip_t *ip_addr_ptr )
{
     if ( ip_addr_ptr->family == bdmf_ip_family_ipv4 )
     {
         if ( ip_addr_ptr->addr.ipv4 )
            return 1;
         else
             return 0;
     }
     else if ( ip_addr_ptr->family == bdmf_ip_family_ipv6 )
     {
        if ( ip_addr_ptr->addr.ipv6.data[0] || ip_addr_ptr->addr.ipv6.data[1] ||
             ip_addr_ptr->addr.ipv6.data[2] || ip_addr_ptr->addr.ipv6.data[3] || 
             ip_addr_ptr->addr.ipv6.data[4] || ip_addr_ptr->addr.ipv6.data[5] || 
             ip_addr_ptr->addr.ipv6.data[6] || ip_addr_ptr->addr.ipv6.data[7] || 
             ip_addr_ptr->addr.ipv6.data[8] || ip_addr_ptr->addr.ipv6.data[9] || 
             ip_addr_ptr->addr.ipv6.data[10] || ip_addr_ptr->addr.ipv6.data[11] || 
             ip_addr_ptr->addr.ipv6.data[12] || ip_addr_ptr->addr.ipv6.data[13] || 
             ip_addr_ptr->addr.ipv6.data[14] || ip_addr_ptr->addr.ipv6.data[15] ) 
            return 1;
         else
            return 0;
     }

     return -1; /* should never reach here */
}


static inline void print_ipv6_addr( bdmf_ipv6_t *ipv6_addr_ptr )
{
    __debug_mcast("0x%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
          ipv6_addr_ptr->data[0], ipv6_addr_ptr->data[1], ipv6_addr_ptr->data[2], ipv6_addr_ptr->data[3], 
          ipv6_addr_ptr->data[4], ipv6_addr_ptr->data[5], ipv6_addr_ptr->data[6], ipv6_addr_ptr->data[7], 
          ipv6_addr_ptr->data[8], ipv6_addr_ptr->data[9], ipv6_addr_ptr->data[10], ipv6_addr_ptr->data[11], 
          ipv6_addr_ptr->data[12], ipv6_addr_ptr->data[13], ipv6_addr_ptr->data[14], ipv6_addr_ptr->data[15] ); 
}

static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection2_entry_lookup ( rdpa_mcast_flow_key_t   *mcast_lookup_entry,
                                                                             uint32_t                xi_ip_sa,
                                                                             uint16_t                xi_vlan_head_index,
                                                                             uint16_t                *xo_connection2_entry_index )
{
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    uint16_t                  connection2_entry_index;
    uint16_t                  mcast_connection2_entry_valid;
    uint16_t                  mcast_connection2_entry_vid0;
    uint16_t                  mcast_connection2_entry_vid1;
    uint8_t                   mcast_connection2_entry_rx_if;
    uint32_t                  mcast_connection2_entry_ip_sa;
    uint16_t                  loop_count = 0;

    __debug_mcast("\n%s, %u: Input Params: vlan_head_index=%u, rx_if=%u, "
                  "vid0=0x%03X vid1=0x%03X\n", __func__, __LINE__, 
                  xi_vlan_head_index, mcast_lookup_entry->rx_if,
                  mcast_lookup_entry->outer_vlan_id,
                  mcast_lookup_entry->inner_vlan_id);

    *xo_connection2_entry_index = RDD_FC_MCAST_CONNECTION2_NEXT_INVALID ;

    connection2_entry_index = xi_vlan_head_index;
    while ( connection2_entry_index != RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
    {
        if ( connection2_entry_index >= RDD_FC_MCAST_CONNECTION2_TABLE_SIZE )
        {
            __debug_mcast("%s, %u: Error!!!: invalid connection2_entry_index value=%d\n", __FUNCTION__, __LINE__, connection2_entry_index);
            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
        }

        mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ connection2_entry_index ] );

        RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_READ ( mcast_connection2_entry_valid, mcast_connection2_entry_ptr );

        if ( !( mcast_connection2_entry_valid ) )
        {
            __debug_mcast("%s, %u: Warning!!!: entry not valid = %d\n", __FUNCTION__, __LINE__, mcast_connection2_entry_valid);
            continue;   /* skip, should not find any invalid entry in the connection2 linked list */
        }

        /* if entry is valid, check if it matches entry being added */
        RDD_FC_MCAST_CONNECTION2_ENTRY_IP_SA_READ ( mcast_connection2_entry_ip_sa, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID0_READ ( mcast_connection2_entry_vid0, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID1_READ ( mcast_connection2_entry_vid1, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_RX_IF_READ ( mcast_connection2_entry_rx_if, mcast_connection2_entry_ptr );

        if ( ( mcast_connection2_entry_ip_sa == xi_ip_sa ) &&
             ( mcast_connection2_entry_rx_if == (uint8_t)mcast_lookup_entry->rx_if ) &&
             ( mcast_connection2_entry_vid0 == mcast_lookup_entry->outer_vlan_id ) &&
             ( mcast_connection2_entry_vid1 == mcast_lookup_entry->inner_vlan_id ) )
        {
            __debug_mcast("%s, %u: match found=%u\n", __FUNCTION__, __LINE__, connection2_entry_index);
            break;
        }
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_READ( connection2_entry_index, mcast_connection2_entry_ptr );

        loop_count++;
        if (loop_count > RDD_FC_MCAST_CONNECTION2_TABLE_SIZE) 
        {
            __debug_mcast("%s, %u: Error!!!: loop_count exceeded\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    *xo_connection2_entry_index = connection2_entry_index;
    return ( BL_LILAC_RDD_OK );
}


static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_lookup ( uint32_t              xi_hash_index,
                                                                            rdpa_mcast_flow_key_t *mcast_lookup_entry,
                                                                            uint32_t              xi_ipv6_src_ip_crc,
                                                                            uint32_t              xi_ipv6_dst_ip_crc,
                                                                            uint32_t              *xo_tries )
{
    RDD_FC_MCAST_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  tries;
    uint32_t                  connection_entry_index;
    uint32_t                  connection_entry_valid;
    uint8_t                   connection_entry_protocol;
    uint8_t                   connection_entry_is_multicast;
    uint8_t                   connection_entry_number_of_tags;
    bdmf_ip_t                 connection_entry_dst_ip;
 
    __debug_mcast("\n%s, %u: ============================================================ \n", __FUNCTION__, __LINE__); 
    __debug_mcast("%s, %u: Input Params: hash_index=%u protocol=%u, num_vlan_tag=%u \n", __FUNCTION__, __LINE__, 
              xi_hash_index, mcast_lookup_entry->protocol, mcast_lookup_entry->num_vlan_tags);

    __debug_mcast("%s, %u: vid0=0x%03X vid1=0x%03X\n", __FUNCTION__, __LINE__, 
              mcast_lookup_entry->outer_vlan_id, mcast_lookup_entry->inner_vlan_id);

    if ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        __debug_mcast("%s, %u: Input Params: IPSA=0x%08X IPDA=0x%08X\n", __FUNCTION__, __LINE__, 
              mcast_lookup_entry->src_ip.addr.ipv4, mcast_lookup_entry->dst_ip.addr.ipv4);
    }
    else
    {
        __debug_mcast("%s, %u: Input Params: IPSA=", __FUNCTION__, __LINE__);
        print_ipv6_addr ( &mcast_lookup_entry->src_ip.addr.ipv6 ); __debug_mcast("\n");

        __debug_mcast("%s, %u: Input Params: IPDA=", __FUNCTION__, __LINE__);
        print_ipv6_addr ( &mcast_lookup_entry->dst_ip.addr.ipv6 ); __debug_mcast("\n");

        __debug_mcast("%s, %u: Input Params: ipv6_src_ip_crc=0x%08X ipv6_dst_ip_crc=0x%08X\n", __FUNCTION__, __LINE__, 
                  xi_ipv6_src_ip_crc, xi_ipv6_dst_ip_crc);
    }

    for ( tries = 0; tries < LILAC_RDD_CONNECTION_TABLE_SET_SIZE; tries++ )
    {
        connection_entry_index = xi_hash_index + tries;
        connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[ connection_entry_index ] );
        RDD_FC_MCAST_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );

        if ( connection_entry_valid )
        {
            RDD_FC_MCAST_CONNECTION_ENTRY_IS_MULTICAST_READ ( connection_entry_is_multicast, connection_entry_ptr );
            RDD_FC_MCAST_CONNECTION_ENTRY_PROTOCOL_READ ( connection_entry_protocol, connection_entry_ptr );
            RDD_FC_MCAST_CONNECTION_ENTRY_NUMBER_OF_TAGS_READ ( connection_entry_number_of_tags, connection_entry_ptr );
            RDD_FC_MCAST_CONNECTION_ENTRY_DST_IP_READ ( connection_entry_dst_ip.addr.ipv4, connection_entry_ptr );

            if ( ( connection_entry_is_multicast == LILAC_RDD_TRUE ) &&
                 ( connection_entry_number_of_tags == mcast_lookup_entry->num_vlan_tags ) &&
                 ( connection_entry_protocol == (mcast_lookup_entry->protocol & 0x7F) ) &&
                 ( ( ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 ) &&
                     ( connection_entry_dst_ip.addr.ipv4 == mcast_lookup_entry->dst_ip.addr.ipv4 ) ) ||
                   ( ( ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv6 ) &&
                       ( connection_entry_dst_ip.addr.ipv4 == xi_ipv6_dst_ip_crc ) ) ) ) )
            {
                break;
            }
        }
    }

    *xo_tries = tries;
    __debug_mcast("%s, %u: tries=%u\n", __FUNCTION__, __LINE__, tries); 

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_search(rdd_mcast_flow_t *rdd_mcast_flow,
                                                            bdmf_index       *xo_entry_index )
{
    RDD_FC_MCAST_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint8_t                   entry_bytes[ LILAC_RDD_CONNECTION_ENTRY_SIZE ];
    uint32_t                  crc_init_value, crc_result, hash_index, tries;
    uint32_t                  connection_entry_index;
    uint32_t                  context_index;
    uint32_t                  ipv6_src_ip_crc;
    uint32_t                  ipv6_dst_ip_crc;
    uint32_t                  ip_sa;
    rdpa_mcast_flow_key_t     *mcast_lookup_entry = &rdd_mcast_flow->key;
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    uint16_t                  connection2_entry_index ;
    uint16_t                  vlan_head_index;
    uint32_t                  ssm=0;
    unsigned long             flags;

    __debug_mcast("\n%s, %u: ============================================================ \n", __FUNCTION__, __LINE__); 
    __debug_mcast("%s, %u: Input Params: protocol=%u, num_vlan_tag=%u \n", __FUNCTION__, __LINE__, 
              mcast_lookup_entry->protocol, mcast_lookup_entry->num_vlan_tags);

    __debug_mcast("%s, %u: vid0=0x%03X vid1=0x%03X\n", __FUNCTION__, __LINE__, 
              mcast_lookup_entry->outer_vlan_id, mcast_lookup_entry->inner_vlan_id);

    if ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        __debug_mcast("%s, %u: Input Params: IPSA=0x%08X IPDA=0x%08X\n", __FUNCTION__, __LINE__, 
              mcast_lookup_entry->src_ip.addr.ipv4, mcast_lookup_entry->dst_ip.addr.ipv4);
    }
    else
    {
        __debug_mcast("%s, %u: Input Params: IPSA=", __FUNCTION__, __LINE__);
        print_ipv6_addr ( &mcast_lookup_entry->src_ip.addr.ipv6 ); __debug_mcast("\n");

        __debug_mcast("%s, %u: Input Params: IPDA=", __FUNCTION__, __LINE__);
        print_ipv6_addr ( &mcast_lookup_entry->dst_ip.addr.ipv6 ); __debug_mcast("\n");
    }

    if ( ( mcast_lookup_entry->num_vlan_tags > 2 ) ||
         ( mcast_lookup_entry->inner_vlan_id > 0xFFF ) ||
         ( mcast_lookup_entry->outer_vlan_id > 0xFFF ) )
    {
        __debug_mcast("%s, %u: invalid params\n", __FUNCTION__, __LINE__); 
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    ipv6_src_ip_crc = 0;
    ipv6_dst_ip_crc = 0;

    entry_bytes[ 0 ] = 0;
    entry_bytes[ 1 ] = 0;
    entry_bytes[ 2 ] = 0;
    entry_bytes[ 3 ] = 0;
    entry_bytes[ 3 ] = mcast_lookup_entry->protocol | (1 << 7);
    entry_bytes[ 4 ] = 0; /* DSL */
    entry_bytes[ 5 ] = 0;
    entry_bytes[ 6 ] = 0;
    entry_bytes[ 7 ] = ( mcast_lookup_entry->num_vlan_tags & 0x03 ) << 6; /* DSL */

    if ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        ip_sa = mcast_lookup_entry->src_ip.addr.ipv4;

        entry_bytes[ 12 ] = ( mcast_lookup_entry->dst_ip.addr.ipv4 >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( mcast_lookup_entry->dst_ip.addr.ipv4 >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( mcast_lookup_entry->dst_ip.addr.ipv4 >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = mcast_lookup_entry->dst_ip.addr.ipv4 & 0xFF;
    }
    else
    {
        /* In case of SSM, IPSA should be 0 */
        if ( is_ip_addr_non_zero( &mcast_lookup_entry->src_ip ) )
            ssm = 1;

        if (ssm)
            ipv6_src_ip_crc = crcbitbybit((uint8_t *)&mcast_lookup_entry->src_ip.addr.ipv6.data,
                                                 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        ip_sa = ipv6_src_ip_crc; 
        ipv6_dst_ip_crc = crcbitbybit((uint8_t *)&mcast_lookup_entry->dst_ip.addr.ipv6.data,
                                                 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        entry_bytes[ 12 ] = ( ipv6_dst_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( ipv6_dst_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( ipv6_dst_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( ipv6_dst_ip_crc >> 0 ) & 0xFF;
    }

    /* calculate the CRC on the connection entry: num_vlang_tags + MCAST IPDA */
    crc_init_value = mcast_lookup_entry->num_vlan_tags;
    crc_result = crcbitbybit ( &entry_bytes[ 12 ], 4, 0, crc_init_value, RDD_CRC_TYPE_32 );
    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );
    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    rdd_fc_mcast_connection_entry_lookup ( hash_index, mcast_lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );
    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        rdd_fc_mcast_connection_entry_lookup ( hash_index, mcast_lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );
        if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            __debug_mcast("%s, %u: search failed\n", __FUNCTION__, __LINE__); 
            bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
        }
    }

    connection_entry_index = hash_index + tries;
    connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[ connection_entry_index ] );

    RDD_FC_MCAST_CONNECTION_ENTRY_VLAN_HEAD_INDEX_READ( vlan_head_index, connection_entry_ptr );
    rdd_fc_mcast_connection2_entry_lookup ( mcast_lookup_entry, ip_sa, vlan_head_index, &connection2_entry_index ) ;
    if ( connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
    {
        __debug_mcast("%s, %u: end of list: connection2_entry_index=%u\n", __FUNCTION__, __LINE__, connection2_entry_index); 
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ connection2_entry_index ] );
    RDD_FC_MCAST_CONNECTION2_ENTRY_CONTEXT_INDEX_READ ( context_index, mcast_connection2_entry_ptr );
    *xo_entry_index = context_index;

    __debug_mcast("%s, %u: search successful: context_entry=%u\n", __FUNCTION__, __LINE__, *xo_entry_index); 
    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


/* get mcast connection2 entry by lookup entry */
static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection2_entry_get ( uint16_t                  xi_vlan_head_index,
                                                                          uint32_t                  xi_context_entry_index,
                                                                          uint16_t                  *xo_connection2_entry_index )
{
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    uint16_t                  connection2_entry_index, next_entry_index;
    uint32_t                  mcast_connection2_entry_valid;
#if defined(CC_RDD_ROUTER_DEBUG)
    uint16_t                  mcast_connection2_entry_vid0;
    uint16_t                  mcast_connection2_entry_vid1;
    uint32_t                  mcast_connection2_entry_ip_sa;
    uint8_t                   mcast_connection2_entry_rx_if;
#endif
    uint16_t                  mcast_connection2_entry_context_index;
    uint16_t                  loop_count = 0;

    __debug_mcast("\n%s, %u: Input Params: head_index=%u, context_index=%u\n", __FUNCTION__, __LINE__, 
              xi_vlan_head_index, xi_context_entry_index);

    next_entry_index = connection2_entry_index = xi_vlan_head_index;
    while ( connection2_entry_index != RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
    {   
        if ( connection2_entry_index >= RDD_FC_MCAST_CONNECTION2_TABLE_SIZE )
        {
            __debug_mcast("%s, %u: Error!!!: invalid connection2_entry_index value=%d\n", __FUNCTION__, __LINE__, connection2_entry_index);
            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
        }

        mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ connection2_entry_index ] );

        RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_READ ( mcast_connection2_entry_valid, mcast_connection2_entry_ptr );

        if ( !( mcast_connection2_entry_valid ) )
        {
            __debug_mcast("%s, %u: Warning!!!: entry not valid = %d\n", __FUNCTION__, __LINE__, mcast_connection2_entry_valid);
            *xo_connection2_entry_index = connection2_entry_index;
            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
        }

        /* if entry is valid, check if it matches entry */
#if defined(CC_RDD_ROUTER_DEBUG)
        RDD_FC_MCAST_CONNECTION2_ENTRY_IP_SA_READ ( mcast_connection2_entry_ip_sa, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID0_READ ( mcast_connection2_entry_vid0, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID1_READ ( mcast_connection2_entry_vid1, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_RX_IF_READ ( mcast_connection2_entry_rx_if, mcast_connection2_entry_ptr );
#endif

        RDD_FC_MCAST_CONNECTION2_ENTRY_CONTEXT_INDEX_READ ( mcast_connection2_entry_context_index, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_READ( next_entry_index, mcast_connection2_entry_ptr );

        __debug_mcast("%s, %u: conn entry connection2_entry_index=%u valid=%u "
                      "IPSA=0x%08X rx_if=%u vid0=0x%03X vid1=0x%03X "
                      "context_index=%u next=%u\n", __func__, __LINE__,
                      connection2_entry_index, mcast_connection2_entry_valid,
                      mcast_connection2_entry_ip_sa, mcast_connection2_entry_rx_if,
                      mcast_connection2_entry_vid0, mcast_connection2_entry_vid1,
                      mcast_connection2_entry_context_index, next_entry_index);

        if ( mcast_connection2_entry_context_index == xi_context_entry_index)
        {
            __debug_mcast("%s, %u: match found connection2_entry_index=%u\n", __FUNCTION__, __LINE__, connection2_entry_index);
            break;
        }

        connection2_entry_index = next_entry_index;

        loop_count++;
        if (loop_count > RDD_FC_MCAST_CONNECTION2_TABLE_SIZE) 
        {
            __debug_mcast("%s, %u: Error!!!: loop_count exceeded\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    *xo_connection2_entry_index = connection2_entry_index;
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_get ( uint32_t                 xi_connection_entry_index,
                                                           bdmf_index               xi_context_entry_index,
                                                           rdpa_mcast_flow_key_t    *mcast_lookup_entry)
{
    RDD_FC_MCAST_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  connection_entry_valid;
    uint32_t                  connection_entry_is_multicast;
    BL_LILAC_RDD_ERROR_DTE    rdd_error;
    uint16_t                  connection2_entry_index;
    uint16_t                  vlan_head_index;
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;

    __debug_mcast("\n%s, %u: ============================================================ \n", __FUNCTION__, __LINE__); 
    __debug_mcast("%s, %u: Input Params: connection_entry_index=%u, context_entry_index=%u\n", 
        __FUNCTION__, __LINE__, xi_connection_entry_index, xi_context_entry_index);

    if ( xi_connection_entry_index >= RDD_CONNECTION_TABLE_SIZE )
    {
        __debug_mcast("%s, %u: Error!!!: invalid xi_connection_entry_index=%u\n", __FUNCTION__, __LINE__, xi_connection_entry_index);
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[ xi_connection_entry_index ] );

    RDD_FC_MCAST_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );
    RDD_FC_MCAST_CONNECTION_ENTRY_IS_MULTICAST_READ ( connection_entry_is_multicast, connection_entry_ptr );

    if ( ( !connection_entry_valid ) || ( !connection_entry_is_multicast ) )
    {
        __debug_mcast("%s, %u: Error!!!: entry not valid = %d or not multicast\n", __FUNCTION__, __LINE__, connection_entry_valid);
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_FC_MCAST_CONNECTION_ENTRY_PROTOCOL_READ ( mcast_lookup_entry->protocol, connection_entry_ptr );
    mcast_lookup_entry->protocol &= 0x7F;
    RDD_FC_MCAST_CONNECTION_ENTRY_NUMBER_OF_TAGS_READ ( mcast_lookup_entry->num_vlan_tags, connection_entry_ptr );
    RDD_FC_MCAST_CONNECTION_ENTRY_DST_IP_READ ( mcast_lookup_entry->dst_ip.addr.ipv4, connection_entry_ptr );
    RDD_FC_MCAST_CONNECTION_ENTRY_KEY_EXTEND_READ ( mcast_lookup_entry->dst_ip.family, connection_entry_ptr );
    RDD_FC_MCAST_CONNECTION_ENTRY_VLAN_HEAD_INDEX_READ( vlan_head_index, connection_entry_ptr );

    rdd_error = rdd_fc_mcast_connection2_entry_get ( vlan_head_index, xi_context_entry_index, &connection2_entry_index );

    if ( ( rdd_error != BL_LILAC_RDD_OK ) || ( connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID ) )
    {
        __debug_mcast("%s, %u: Error!!!: rdd_error=%u invalid connection2_entry_index value=%d\n",
            __FUNCTION__, __LINE__, rdd_error, connection2_entry_index);
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ connection2_entry_index ] );

    RDD_FC_MCAST_CONNECTION2_ENTRY_VID0_READ(mcast_lookup_entry->outer_vlan_id, mcast_connection2_entry_ptr);
    RDD_FC_MCAST_CONNECTION2_ENTRY_VID1_READ(mcast_lookup_entry->inner_vlan_id, mcast_connection2_entry_ptr);
    RDD_FC_MCAST_CONNECTION2_ENTRY_RX_IF_READ(mcast_lookup_entry->rx_if, mcast_connection2_entry_ptr);
    RDD_FC_MCAST_CONNECTION2_ENTRY_IP_SA_READ(mcast_lookup_entry->src_ip.addr.ipv4, mcast_connection2_entry_ptr);
    return ( rdd_error );
}


/* finds out the free mcast connection2 entry */
static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection2_entry_alloc(uint16_t *xo_connection2_entry_index)
{
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    uint16_t                  connection2_entry_index;
    uint16_t                  mcast_connection2_entry_valid;

    for ( connection2_entry_index = 0; connection2_entry_index < RDD_FC_MCAST_CONNECTION2_TABLE_SIZE; connection2_entry_index++ )
    {
        mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ connection2_entry_index ] );

        RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_READ ( mcast_connection2_entry_valid, mcast_connection2_entry_ptr );

        if ( !( mcast_connection2_entry_valid ) )
        {
            break;  /* found unused entry */
        }
    }

    *xo_connection2_entry_index = connection2_entry_index;
    return ( BL_LILAC_RDD_OK );
}


static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_alloc ( uint32_t              xi_hash_index,
                                                                          rdpa_mcast_flow_key_t *xi_lookup_entry,
                                                                          uint32_t              xi_ipv6_src_ip_crc,
                                                                          uint32_t              xi_ipv6_dst_ip_crc,
                                                                          uint32_t              *xo_tries )
{
    RDD_FC_MCAST_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  tries;
    uint32_t                  connection_entry_index;
    uint32_t                  connection_entry_valid;

    for ( tries = 0; tries < LILAC_RDD_CONNECTION_TABLE_SET_SIZE; tries++ )
    {
        connection_entry_index = xi_hash_index + tries;
        connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[ connection_entry_index ] );

        RDD_FC_MCAST_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );

        if ( !( connection_entry_valid ) )
        {
            break; /* found an unused entry */
        }
    }
    *xo_tries = tries;
    return ( BL_LILAC_RDD_OK );
}


#if defined(CC_RDD_ROUTER_DEBUG)
/* Enter the new connection2 entry in sorted order of VIDs */
static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection2_entry_dump ( uint16_t  xi_vlan_head_index )
{
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    uint32_t                  mcast_connection2_entry_valid;
    uint16_t                  mcast_connection2_entry_vid0;
    uint16_t                  mcast_connection2_entry_vid1;
    uint8_t                   mcast_connection2_entry_rx_if;
    uint16_t                  cur_connection2_entry_index;
    uint16_t                  prev_connection2_entry_index;
    uint16_t                  next_connection2_entry_index;
    uint16_t                  mcast_connection2_entry_context_index;
    uint32_t                  mcast_connection2_entry_ip_sa;
    uint16_t                  loop_count = 0;

    __debug_mcast("%s, %u: dump params head_index=%u \n", __FUNCTION__, __LINE__, xi_vlan_head_index);

    cur_connection2_entry_index  = xi_vlan_head_index;
    prev_connection2_entry_index = RDD_FC_MCAST_CONNECTION2_NEXT_INVALID;
    next_connection2_entry_index = RDD_FC_MCAST_CONNECTION2_NEXT_INVALID;

    /* Find the entry next in the sorted order to the searched entry */
    while ( cur_connection2_entry_index != RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
    {
        if ( cur_connection2_entry_index >= RDD_FC_MCAST_CONNECTION2_TABLE_SIZE )
        {
            __debug_mcast("%s, %u: Error!!!: invalid connection2_entry_index value=%d\n", __FUNCTION__, __LINE__, cur_connection2_entry_index);
            return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
        }

        mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ cur_connection2_entry_index ] );

        RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_READ ( mcast_connection2_entry_valid, mcast_connection2_entry_ptr );

        if ( !( mcast_connection2_entry_valid ) )
        {
            __debug_mcast("%s, %u: Warning!!!: entry not valid = %d\n", __FUNCTION__, __LINE__, mcast_connection2_entry_valid);
            /* A list cannot have invalid entries */
            return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
        }

        RDD_FC_MCAST_CONNECTION2_ENTRY_IP_SA_READ ( mcast_connection2_entry_ip_sa, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID0_READ ( mcast_connection2_entry_vid0, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID1_READ ( mcast_connection2_entry_vid1, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_RX_IF_READ ( mcast_connection2_entry_rx_if, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_CONTEXT_INDEX_READ ( mcast_connection2_entry_context_index, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_READ( next_connection2_entry_index, mcast_connection2_entry_ptr );

        __debug_mcast("%s, %u: cur=%u prev=%u next=%u valid=%u IPSA=0x%08X "
                      "rx_if=%u vid0=0x%03X vid1=0x%03X context_index=%u\n", __func__, __LINE__, 
                      cur_connection2_entry_index, prev_connection2_entry_index,
                      next_connection2_entry_index, mcast_connection2_entry_valid,
                      mcast_connection2_entry_ip_sa, mcast_connection2_entry_rx_if,
                      mcast_connection2_entry_vid0, mcast_connection2_entry_vid1, 
                      mcast_connection2_entry_context_index);

        loop_count++;
        if (loop_count > RDD_FC_MCAST_CONNECTION2_TABLE_SIZE) 
        {
            __debug_mcast("%s, %u: Error!!!: loop_count exceeded\n", __FUNCTION__, __LINE__);
            break;
        }

        prev_connection2_entry_index = cur_connection2_entry_index;
        cur_connection2_entry_index = next_connection2_entry_index;
    }

    return ( BL_LILAC_RDD_OK );
}
#else
static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection2_entry_dump(uint16_t xi_vlan_head_index)
{
    return (BL_LILAC_RDD_OK);
}
#endif


/* Enter the new connection2 entry in sorted order */
static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection2_entry_add ( rdpa_mcast_flow_key_t     *mcast_lookup_entry,
                                                                          uint16_t                  xi_vlan_head_index,
                                                                          uint32_t                  xi_ip_sa,
                                                                          uint16_t                  xi_connection2_alloc_index,
                                                                          uint32_t                  xi_context_entry_index,
                                                                          uint16_t                  *xo_vlan_head_index)
{
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *prev_mcast_connection2_entry_ptr;
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *alloc_mcast_connection2_entry_ptr;
    uint32_t                  mcast_connection2_entry_valid;
    uint16_t                  mcast_connection2_entry_vid0;
    uint16_t                  mcast_connection2_entry_vid1;
    uint8_t                   mcast_connection2_entry_rx_if;
    uint32_t                  mcast_connection2_entry_ip_sa;
    uint16_t                  cur_connection2_entry_index;
    uint16_t                  prev_connection2_entry_index;
    uint16_t                  next_connection2_entry_index;
    uint16_t                  loop_count = 0;

    __debug_mcast("\n%s, %u: Input Params: head_index=%u, alloc_index=%u, IPSA=0x%08X "
                  "rx_if = %u vid0=0x%03X vid1=0x%03X\n", __func__, __LINE__, 
                  xi_vlan_head_index, xi_connection2_alloc_index, xi_ip_sa,
                  mcast_lookup_entry->rx_if, mcast_lookup_entry->outer_vlan_id,
                  mcast_lookup_entry->inner_vlan_id);

    *xo_vlan_head_index = xi_vlan_head_index;

    if ( xi_connection2_alloc_index >= RDD_FC_MCAST_CONNECTION2_TABLE_SIZE )
    {
        __debug_mcast("%s, %u: Error!!!: invalid connection2_entry_index value=%d\n", __FUNCTION__, __LINE__, xi_connection2_alloc_index);
        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    cur_connection2_entry_index = xi_vlan_head_index;
    prev_connection2_entry_index = RDD_FC_MCAST_CONNECTION2_NEXT_INVALID;
    next_connection2_entry_index = RDD_FC_MCAST_CONNECTION2_NEXT_INVALID;

    /* Find the entry next in the sorted order to the searched entry */
    while ( cur_connection2_entry_index != RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
    {
        if ( cur_connection2_entry_index >= RDD_FC_MCAST_CONNECTION2_TABLE_SIZE )
        {
            __debug_mcast("%s, %u: Error!!!: invalid connection2_entry_index value=%d\n", __FUNCTION__, __LINE__, cur_connection2_entry_index);
            return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
        }

        mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ cur_connection2_entry_index ] );

        RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_READ ( mcast_connection2_entry_valid, mcast_connection2_entry_ptr );

        if ( !( mcast_connection2_entry_valid ) )
        {
            __debug_mcast("%s, %u: Warning!!!: entry not valid = %d\n", __FUNCTION__, __LINE__, mcast_connection2_entry_valid);
            /* A list cannot have invalid entries */
            return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
        }

        RDD_FC_MCAST_CONNECTION2_ENTRY_IP_SA_READ(mcast_connection2_entry_ip_sa, mcast_connection2_entry_ptr);
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_READ(next_connection2_entry_index, mcast_connection2_entry_ptr);
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID0_READ(mcast_connection2_entry_vid0, mcast_connection2_entry_ptr);
        RDD_FC_MCAST_CONNECTION2_ENTRY_VID1_READ(mcast_connection2_entry_vid1, mcast_connection2_entry_ptr);
        RDD_FC_MCAST_CONNECTION2_ENTRY_RX_IF_READ(mcast_connection2_entry_rx_if, mcast_connection2_entry_ptr);

        if ((mcast_connection2_entry_ip_sa == xi_ip_sa) &&
            (mcast_connection2_entry_rx_if == (uint8_t)mcast_lookup_entry->rx_if) &&
            ((mcast_connection2_entry_vid0 > mcast_lookup_entry->outer_vlan_id) ||
             ((mcast_connection2_entry_vid0 == mcast_lookup_entry->outer_vlan_id) &&
              (mcast_connection2_entry_vid1 > mcast_lookup_entry->inner_vlan_id))))
        {
            break; /* found */
        }

#if defined(CC_RDD_ROUTER_DEBUG)
        {
            uint16_t mcast_connection2_entry_context_index;

            RDD_FC_MCAST_CONNECTION2_ENTRY_CONTEXT_INDEX_READ(mcast_connection2_entry_context_index, mcast_connection2_entry_ptr);

            __debug_mcast("%s, %u: entry cur=%u next=%u valid=%u IPSA=0x%08X rx_if=%u "
                          "vid0=0x%03X vid1=0x%03X context_index=%u\n", __func__, __LINE__, 
                          cur_connection2_entry_index, next_connection2_entry_index,
                          mcast_connection2_entry_valid, mcast_connection2_entry_ip_sa,
                          mcast_connection2_entry_rx_if, mcast_connection2_entry_vid0,
                          mcast_connection2_entry_vid1, mcast_connection2_entry_context_index);
        }
#endif

        loop_count++;
        if (loop_count > RDD_FC_MCAST_CONNECTION2_TABLE_SIZE) 
        {
            __debug_mcast("%s, %u: Error!!!: loop_count exceeded\n", __FUNCTION__, __LINE__);
            break;
        }

        prev_connection2_entry_index = cur_connection2_entry_index;
        cur_connection2_entry_index = next_connection2_entry_index;
    }

    /* Fill the entry to be added */
    alloc_mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ xi_connection2_alloc_index ] );
    RDD_FC_MCAST_CONNECTION2_ENTRY_IP_SA_WRITE ( xi_ip_sa, alloc_mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_VID0_WRITE ( mcast_lookup_entry->outer_vlan_id, alloc_mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_VID1_WRITE ( mcast_lookup_entry->inner_vlan_id, alloc_mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_RX_IF_WRITE ( mcast_lookup_entry->rx_if, alloc_mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_CONTEXT_INDEX_WRITE( xi_context_entry_index, alloc_mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE, alloc_mcast_connection2_entry_ptr );

    if (xi_vlan_head_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID)
    {
        /* list is empty, insert at the head */
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( RDD_FC_MCAST_CONNECTION2_NEXT_INVALID, alloc_mcast_connection2_entry_ptr );
        __debug_mcast("%s, %u: Added at the head\n", __FUNCTION__, __LINE__ );
    }
    else if ( (xi_vlan_head_index != RDD_FC_MCAST_CONNECTION2_NEXT_INVALID) && (prev_connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID) )
    {
        /* insert at the head */
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( cur_connection2_entry_index, alloc_mcast_connection2_entry_ptr );
        *xo_vlan_head_index = xi_connection2_alloc_index;
        __debug_mcast("%s, %u: Added at the head new_head_ndex=%u\n", __FUNCTION__, __LINE__, *xo_vlan_head_index );
    }
    else if ( (next_connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID) && (cur_connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID) )
    {
        /* insert at the end of list */
        prev_mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ prev_connection2_entry_index ] );
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( xi_connection2_alloc_index, prev_mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( RDD_FC_MCAST_CONNECTION2_NEXT_INVALID, alloc_mcast_connection2_entry_ptr );
        __debug_mcast("%s, %u: Added at the end alloc_index=%u, cur=%u prev=%u, next=%u\n", __FUNCTION__, __LINE__, 
                xi_connection2_alloc_index, cur_connection2_entry_index, prev_connection2_entry_index, next_connection2_entry_index);
    }
    else
    {
        /* insert at middle of list */
        prev_mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ prev_connection2_entry_index ] );
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( xi_connection2_alloc_index, prev_mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( cur_connection2_entry_index, alloc_mcast_connection2_entry_ptr );
        __debug_mcast("%s, %u: Added at the middle\n", __FUNCTION__, __LINE__ );
    }

    __debug_mcast("%s, %u: Added mcast connection2 entry alloc_index=%u, cur=%u prev=%u, next=%u\n", __FUNCTION__, __LINE__, 
            xi_connection2_alloc_index, cur_connection2_entry_index, prev_connection2_entry_index, next_connection2_entry_index);
    __debug_mcast("%s, %u: alloc_mcast_connection2_entry_ptr = 0x%08x\n", __FUNCTION__, __LINE__, (unsigned int) alloc_mcast_connection2_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_add(rdd_mcast_flow_t *rdd_mcast_flow)
{
    RDD_FC_MCAST_CONNECTION_ENTRY_DTS     *connection_entry_ptr;
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint8_t                      entry_bytes[ LILAC_RDD_CONNECTION_ENTRY_SIZE ];
    uint32_t                     crc_init_value, crc_result, hash_index, tries, saved_hash_index;
    uint32_t                     connection_entry_index;
    uint32_t                     context_entry_index;
    uint32_t                     ipv6_src_ip_crc;
    uint32_t                     ipv6_dst_ip_crc;
    uint32_t                     ip_sa;
    BL_LILAC_RDD_ERROR_DTE       rdd_error;
    uint32_t                     bucket_overflow_counter;
    uint32_t                     entry_overflow;
    uint16_t                     connection2_entry_index;
    uint16_t                     vlan_head_index, new_vlan_head_index;
    uint32_t                     connection_entry_exists = LILAC_RDD_FALSE;
    rdpa_mcast_flow_key_t        *mcast_lookup_entry = &rdd_mcast_flow->key;
    uint32_t                     ssm=0;
    unsigned long                flags;

    __debug_mcast("\n%s, %u: ============================================================ \n", __func__, __LINE__); 
    __debug_mcast("%s, %u: Input Params: rx_if = %d, protocol=%u, num_vlan_tag=%u \n", __func__, __LINE__, 
              mcast_lookup_entry->rx_if, mcast_lookup_entry->protocol, mcast_lookup_entry->num_vlan_tags);

    __debug_mcast("%s, %u: vid0=0x%03X vid1=0x%03X\n", __FUNCTION__, __LINE__, 
              mcast_lookup_entry->outer_vlan_id, mcast_lookup_entry->inner_vlan_id);

    if ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        __debug_mcast("%s, %u: Input Params: IPSA=0x%08X IPDA=0x%08X\n", __FUNCTION__, __LINE__, 
              mcast_lookup_entry->src_ip.addr.ipv4, mcast_lookup_entry->dst_ip.addr.ipv4);
    }
    else
    {
        __debug_mcast("%s, %u: Input Params: IPSA=", __FUNCTION__, __LINE__);
        print_ipv6_addr ( &mcast_lookup_entry->src_ip.addr.ipv6 ); __debug_mcast("\n");

        __debug_mcast("%s, %u: Input Params: IPDA=", __FUNCTION__, __LINE__);
        print_ipv6_addr ( &mcast_lookup_entry->dst_ip.addr.ipv6 ); __debug_mcast("\n");
    }

    if ( ( mcast_lookup_entry->num_vlan_tags > 2 ) ||
         ( mcast_lookup_entry->inner_vlan_id > 0xFFF ) ||
         ( mcast_lookup_entry->outer_vlan_id > 0xFFF ) )
    {
        __debug_mcast("%s, %u: invalid params\n", __FUNCTION__, __LINE__); 
        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    ipv6_src_ip_crc = 0;
    ipv6_dst_ip_crc = 0;

    entry_bytes[ 0 ] = 0;
    entry_bytes[ 1 ] = 0;
    entry_bytes[ 2 ] = 0;
    entry_bytes[ 3 ] = mcast_lookup_entry->protocol | (1 << 7);
    entry_bytes[ 4 ] = 0;
    entry_bytes[ 5 ] = 0;
    entry_bytes[ 6 ] = 0;
    entry_bytes[ 7 ] = ( mcast_lookup_entry->num_vlan_tags & 0x03 ) << 6;

    if ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
    {
        ip_sa = mcast_lookup_entry->src_ip.addr.ipv4;

        entry_bytes[ 12 ] = ( mcast_lookup_entry->dst_ip.addr.ipv4 >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( mcast_lookup_entry->dst_ip.addr.ipv4 >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( mcast_lookup_entry->dst_ip.addr.ipv4 >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = mcast_lookup_entry->dst_ip.addr.ipv4 & 0xFF;
    }
    else
    {
        /* In case of SSM, IPSA should be 0 */
        if ( is_ip_addr_non_zero( &mcast_lookup_entry->src_ip ) )
            ssm = 1;

        if (ssm)
            ipv6_src_ip_crc = crcbitbybit((uint8_t *)&mcast_lookup_entry->src_ip.addr.ipv6.data,
                                                 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        ip_sa = ipv6_src_ip_crc; 

        ipv6_dst_ip_crc = crcbitbybit((uint8_t *)&mcast_lookup_entry->dst_ip.addr.ipv6.data,
                                             16, 0, 0xffffffff, RDD_CRC_TYPE_32);
        entry_bytes[ 12 ] = ( ipv6_dst_ip_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( ipv6_dst_ip_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( ipv6_dst_ip_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( ipv6_dst_ip_crc >> 0 ) & 0xFF;
    }

    entry_overflow = LILAC_RDD_FALSE;
    crc_init_value = mcast_lookup_entry->num_vlan_tags;

    /* calculate the CRC on the connection entry: num_vlang_tags + MCAST IPDA */
    crc_result = crcbitbybit ( &entry_bytes[ 12 ], 4, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    /* Search before alloc */
    saved_hash_index = hash_index;
    rdd_fc_mcast_connection_entry_lookup ( hash_index, mcast_lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        rdd_fc_mcast_connection_entry_lookup ( hash_index, mcast_lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );
    }

    /* search failed so the mcast connection entry does not exist, need to alloc new entry */
    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = saved_hash_index;

        rdd_error = rdd_fc_mcast_connection_entry_alloc ( hash_index, mcast_lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries);
        if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

            rdd_error = rdd_fc_mcast_connection_entry_alloc ( hash_index, mcast_lookup_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc, &tries );
            if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
            {
                __debug_mcast("%s, %u: Error!!!: all possible entries for the connection are used up\n", __FUNCTION__, __LINE__);
                bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
                return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_NO_EMPTY_ENTRY );
            }
            else 
                entry_overflow = LILAC_RDD_TRUE;
        }
        __debug_mcast("%s, %u: allocated NEW connection entry\n", __FUNCTION__, __LINE__);
    }
    else
    {
        /* entry already exists */
        connection_entry_exists = LILAC_RDD_TRUE;
        __debug_mcast("%s, %u: using existing connection entry\n", __FUNCTION__, __LINE__);
    }

    rdd_fc_mcast_connection2_entry_alloc ( &connection2_entry_index );
    if ( connection2_entry_index >= RDD_FC_MCAST_CONNECTION2_TABLE_SIZE - 1 )
    {
        /* no free entry in mcast vlan table */
        /* Reserve the last entry index */
        __debug_mcast("%s, %u: Error!!!: out of space in mcast vlan table\n", __FUNCTION__, __LINE__);
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_NO_EMPTY_ENTRY );
    }

    if ( entry_overflow == LILAC_RDD_TRUE )
    {
        /*  increment bucket_overflow_counter in the last entry of the previous bucket */
        if ( hash_index == 0 )
        {
            connection_entry_index = RDD_CONNECTION_TABLE_SIZE - 1;
        }
        else
        {
            connection_entry_index = hash_index - 1;
        }

        connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[ connection_entry_index ] );

        RDD_FC_MCAST_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_READ ( bucket_overflow_counter, connection_entry_ptr );
        bucket_overflow_counter++;
        RDD_FC_MCAST_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_WRITE ( bucket_overflow_counter, connection_entry_ptr );
        __debug_mcast("%s, %u: overflow entry bucket_overflow_counter entry=%u\n", __FUNCTION__, __LINE__, bucket_overflow_counter);
    }

    connection_entry_index = hash_index + tries;
    connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[ connection_entry_index ] );

    if (connection_entry_exists != LILAC_RDD_TRUE)
    {
        /* zero out the mcast connection entry */
        MWRITE_32( ((uint32_t *) connection_entry_ptr),  0 );
        MWRITE_32( ((uint32_t *) connection_entry_ptr) + 1, 0 );
        MWRITE_32( ((uint32_t *) connection_entry_ptr) + 2, 0 );
        MWRITE_32( ((uint32_t *) connection_entry_ptr) + 3, 0 );
    }

    rdd_mcast_flow->context.fc_mcast_flow_context_entry.connection_table_index = connection_entry_index;
    rdd_mcast_flow->context.fc_mcast_flow_context_entry.connection_direction = rdpa_dir_ds;
    rdd_mcast_flow->context.fc_mcast_flow_context_entry.multicast_flag = 1;

#if defined(CC_RDD_ROUTER_DEBUG)
    rdd_mcast_flow_dump(&rdd_mcast_flow->context);
#endif

//  rdd_error = rdd_fc_mcast_context_entry_add ( &rdd_mcast_flow->context, &context_entry_index );
    rdd_error = f_rdd_context_entry_add ( &rdd_mcast_flow->context, rdpa_dir_ds, &context_entry_index );
    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        __debug_mcast("%s, %u: Error!!!: context entry=%u add failed\n", __FUNCTION__, __LINE__, context_entry_index);
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    __debug_mcast("%s, %u: connection_entry_index %u, connection_entry_ptr %p, xi_direction %u, context_entry_index %u\n",
            __FUNCTION__, __LINE__, connection_entry_index, connection_entry_ptr, rdpa_dir_ds, context_entry_index);

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
    context_entry_ptr = &(context_table_ptr->entry[ context_entry_index ] );
    __debug_mcast("%s, %u: context_entry_index %u, context_entry_ptr %p, connection_table_index %u\n",
            __FUNCTION__, __LINE__, context_entry_index, context_entry_ptr, 
            context_entry_ptr->fc_mcast_flow_context_entry.connection_table_index );

    if ( entry_overflow == LILAC_RDD_TRUE )
    {
        /* set entry_overflow in the context of the entry */
        context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
        context_entry_ptr = &(context_table_ptr->entry[ context_entry_index ] );

        RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_WRITE ( LILAC_RDD_TRUE, context_entry_ptr );

        __debug_mcast("%s, %u: connection_table_index %u\n", __FUNCTION__, __LINE__, 
                context_entry_ptr->fc_mcast_flow_context_entry.connection_table_index);
    }

    if (connection_entry_exists != LILAC_RDD_TRUE)
    {
        vlan_head_index = connection2_entry_index;

        /* finish update of mcast vlan entry before update of vlan head index */
        rdd_fc_mcast_connection2_entry_add ( mcast_lookup_entry, RDD_FC_MCAST_CONNECTION2_NEXT_INVALID, ip_sa, connection2_entry_index, context_entry_index, &new_vlan_head_index );

        if ( mcast_lookup_entry->dst_ip.family == bdmf_ip_family_ipv4 )
        {
            RDD_FC_MCAST_CONNECTION_ENTRY_DST_IP_WRITE ( mcast_lookup_entry->dst_ip.addr.ipv4, connection_entry_ptr );
        }
        else
        {
            RDD_FC_MCAST_CONNECTION_ENTRY_DST_IP_WRITE ( ipv6_dst_ip_crc ,connection_entry_ptr );
        }

        RDD_FC_MCAST_CONNECTION_ENTRY_IS_MULTICAST_WRITE ( LILAC_RDD_ON, connection_entry_ptr );
        RDD_FC_MCAST_CONNECTION_ENTRY_PROTOCOL_WRITE ( (mcast_lookup_entry->protocol | (1<<7)), connection_entry_ptr );
        RDD_FC_MCAST_CONNECTION_ENTRY_NUMBER_OF_TAGS_WRITE ( mcast_lookup_entry->num_vlan_tags, connection_entry_ptr );
        RDD_FC_MCAST_CONNECTION_ENTRY_COMMAND_LIST_LENGTH_64_WRITE( rdd_mcast_flow->context.fc_mcast_flow_context_entry.command_list_length_64, connection_entry_ptr );
        RDD_FC_MCAST_CONNECTION_ENTRY_KEY_EXTEND_WRITE ( mcast_lookup_entry->dst_ip.family, connection_entry_ptr );
        RDD_FC_MCAST_CONNECTION_ENTRY_VLAN_HEAD_INDEX_WRITE( connection2_entry_index, connection_entry_ptr );
        RDD_FC_MCAST_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_ON, connection_entry_ptr );
        rdd_fc_mcast_connection2_entry_dump( connection2_entry_index ); 
    }
    else
    {
        
        RDD_FC_MCAST_CONNECTION_ENTRY_VLAN_HEAD_INDEX_READ( vlan_head_index, connection_entry_ptr );
        rdd_fc_mcast_connection2_entry_add ( mcast_lookup_entry, vlan_head_index, ip_sa, connection2_entry_index, 
            context_entry_index, &new_vlan_head_index );
        RDD_FC_MCAST_CONNECTION_ENTRY_VLAN_HEAD_INDEX_WRITE( new_vlan_head_index, connection_entry_ptr );
        rdd_fc_mcast_connection2_entry_dump( new_vlan_head_index ); 
    }

    /* return the index of the entry in the table */
    rdd_mcast_flow->xo_entry_index = context_entry_index;

    __debug_mcast("%s, %u: connection entry=%u context_entry=%u\n", __FUNCTION__, __LINE__, connection_entry_index, context_entry_index);

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


/* delete the mcast vlan entry */
static inline BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection2_entry_delete ( uint16_t  xi_vlan_head_index,
                                                                      uint32_t  xi_context_entry_index,
                                                                      uint16_t  *xo_vlan_head_index )
{
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *prev_mcast_connection2_entry_ptr;
    uint32_t                  mcast_connection2_entry_valid;
    uint16_t                  connection2_entry_index;
    uint16_t                  prev_connection2_entry_index;
    uint16_t                  next_connection2_entry_index;
    uint32_t                  mcast_connection2_entry_context_entry_index;
    uint16_t                  loop_count = 0;

    __debug_mcast("\n%s, %u: Input Params: vlan_head_index=%u, context_entry_index=%u\n", 
        __FUNCTION__, __LINE__, xi_vlan_head_index, xi_context_entry_index);

    *xo_vlan_head_index = xi_vlan_head_index;
    prev_connection2_entry_index = connection2_entry_index = xi_vlan_head_index;
    next_connection2_entry_index = RDD_FC_MCAST_CONNECTION2_NEXT_INVALID;

    /* Find the entry to be deleted */
    while ( connection2_entry_index != RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
    {
        if ( connection2_entry_index >= RDD_FC_MCAST_CONNECTION2_TABLE_SIZE )
        {
            __debug_mcast("%s, %u: Error!!!: invalid connection2_entry_index value=%d\n", __FUNCTION__, __LINE__, connection2_entry_index);
            return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
        }

        mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ connection2_entry_index ] );

        RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_READ ( mcast_connection2_entry_valid, mcast_connection2_entry_ptr );

        if ( !( mcast_connection2_entry_valid ) )
        {
            __debug_mcast("%s, %u: Warning!!!: entry not valid = %d\n", __FUNCTION__, __LINE__, mcast_connection2_entry_valid);
            continue; /* skip. should never be invalid */
        }

        RDD_FC_MCAST_CONNECTION2_ENTRY_CONTEXT_INDEX_READ ( mcast_connection2_entry_context_entry_index, mcast_connection2_entry_ptr );
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_READ( next_connection2_entry_index, mcast_connection2_entry_ptr );

        if ( mcast_connection2_entry_context_entry_index == xi_context_entry_index )
        {
            break; /* found */
        }

        prev_connection2_entry_index = connection2_entry_index;
        connection2_entry_index = next_connection2_entry_index;

        loop_count++;
        if (loop_count > RDD_FC_MCAST_CONNECTION2_TABLE_SIZE) 
        {
            __debug_mcast("%s, %u: Error!!!: loop_count exceeded\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    if ( connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
    {
        __debug_mcast("%s, %u: Error!!!: invalid connection2_entry_index value=%d\n", __FUNCTION__, __LINE__, connection2_entry_index);
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }

    RDD_FC_MCAST_CONNECTION2_ENTRY_VALID_WRITE ( LILAC_RDD_OFF, mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_VID0_WRITE ( 0xFFF, mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_VID1_WRITE ( 0xFFF, mcast_connection2_entry_ptr );
    RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( RDD_FC_MCAST_CONNECTION2_NEXT_INVALID, mcast_connection2_entry_ptr );

    prev_mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ prev_connection2_entry_index ] );

    if ( xi_vlan_head_index == connection2_entry_index ) 
    {
        /* delete from head */
        *xo_vlan_head_index = next_connection2_entry_index;
    }
    else if (next_connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID)
    {
        /* delete from the end of list */
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( RDD_FC_MCAST_CONNECTION2_NEXT_INVALID, prev_mcast_connection2_entry_ptr );
    }
    else
    {
        /* delete at middle */
        RDD_FC_MCAST_CONNECTION2_ENTRY_NEXT_ENTRY_WRITE( next_connection2_entry_index, prev_mcast_connection2_entry_ptr );
    }

    __debug_mcast("%s, %u: mcast vlan entry=%u deleted, new vlan_head_index=%u\n", 
        __FUNCTION__, __LINE__, connection2_entry_index, *xo_vlan_head_index);

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_delete ( bdmf_index  xi_context_entry_index )
{
    RDD_FC_MCAST_CONNECTION_ENTRY_DTS     *connection_entry_ptr;
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint32_t                     connection_entry_valid;
    uint32_t                     mcast_connection2_entry_context_table_index;
    uint32_t                     entry_overflow;
    uint32_t                     context_entry_connection_table_index;
    uint32_t                     connection_entry_index;
    uint32_t                     bucket_overflow_counter;
    RDD_FC_MCAST_CONNECTION2_ENTRY_DTS  *mcast_connection2_entry_ptr;
    uint16_t                     connection2_entry_index;
    uint16_t                     vlan_head_index, new_vlan_head_index;
    BL_LILAC_RDD_ERROR_DTE       rdd_error;
    unsigned long                flags;

    __debug_mcast("\n%s, %u: ============================================================ \n", __FUNCTION__, __LINE__); 
    __debug_mcast("%s, %u: Input Params: context_entry_index=%u\n", __FUNCTION__, __LINE__, xi_context_entry_index);

    if ( xi_context_entry_index >= RDD_CONTEXT_TABLE_SIZE )
    {
        __debug_mcast("%s, %u: context_entry_index=%u is invalid\n", __FUNCTION__, __LINE__, xi_context_entry_index);
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );
    }

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    context_entry_ptr = &( context_table_ptr->entry[ xi_context_entry_index ] );

    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ ( context_entry_connection_table_index, context_entry_ptr );

    connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[context_entry_connection_table_index] );

    RDD_FC_MCAST_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );

    if ( !connection_entry_valid )
    {
        __debug_mcast("%s, %u: entry not valid=%u\n", __FUNCTION__, __LINE__, connection_entry_valid);

        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }

    RDD_FC_MCAST_CONNECTION_ENTRY_VLAN_HEAD_INDEX_READ( vlan_head_index, connection_entry_ptr );

    rdd_error = rdd_fc_mcast_connection2_entry_get ( vlan_head_index, xi_context_entry_index, &connection2_entry_index );
    if ( ( rdd_error != BL_LILAC_RDD_OK ) || ( connection2_entry_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID ) )
    {
        __debug_mcast("%s, %u: Error!!!: rdd_error=%d connection2_entry_index=%u\n", __FUNCTION__, __LINE__, rdd_error, connection2_entry_index);
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }

    mcast_connection2_entry_ptr = &( g_fc_mcast_connection2_table_ptr->entry[ connection2_entry_index ] );
    RDD_FC_MCAST_CONNECTION2_ENTRY_CONTEXT_INDEX_READ ( mcast_connection2_entry_context_table_index, mcast_connection2_entry_ptr );

    if ( mcast_connection2_entry_context_table_index == xi_context_entry_index )
    {
        rdd_error = rdd_fc_mcast_connection2_entry_delete ( vlan_head_index, xi_context_entry_index, &new_vlan_head_index );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            __debug_mcast("%s, %u: Error!!!: rdd_error=%d\n", __FUNCTION__, __LINE__, rdd_error);
            bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_OK );
        }

        if (vlan_head_index != new_vlan_head_index)
        {
            /* head of the list was deleted, update the vlan head index */
            RDD_FC_MCAST_CONNECTION_ENTRY_VLAN_HEAD_INDEX_WRITE( new_vlan_head_index, connection_entry_ptr );
        }

        if (new_vlan_head_index == RDD_FC_MCAST_CONNECTION2_NEXT_INVALID )
        {
            /* last mcast vlan entry of connection is deleted, so we can delete context and connection entries */
            RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_READ ( entry_overflow, context_entry_ptr );

            if ( entry_overflow == LILAC_RDD_TRUE )
            {
                /* decrement bucket_overflow_counter in the last entry of the previous bucket */
                if ( context_entry_connection_table_index < LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
                {
                    connection_entry_index = RDD_CONNECTION_TABLE_SIZE - 1;
                }
                else
                {
                    connection_entry_index = context_entry_connection_table_index - 
                        ( context_entry_connection_table_index % LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) - 1;
                }
        
                connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) &( g_ds_connection_table_ptr->entry[ connection_entry_index ] );
        
                RDD_FC_MCAST_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_READ ( bucket_overflow_counter, connection_entry_ptr );
                bucket_overflow_counter--;
                RDD_FC_MCAST_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_WRITE ( bucket_overflow_counter, connection_entry_ptr );

                connection_entry_ptr = (RDD_FC_MCAST_CONNECTION_ENTRY_DTS *) 
                    &( g_ds_connection_table_ptr->entry[ context_entry_connection_table_index ] );
            }

            RDD_FC_MCAST_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_OFF, connection_entry_ptr );
            __debug_mcast("%s, %u: mcast connection entry=%u deleted.\n", __FUNCTION__, __LINE__, context_entry_connection_table_index);
        }

        f_rdd_free_context_entry ( xi_context_entry_index );
    }
    else
    {
        __debug_mcast("%s, %u: Error!!!: delete failed. context entry mistmatch\n", __FUNCTION__, __LINE__);
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }

    __debug_mcast("%s, %u: mcast context entry=%u deleted.\n", __FUNCTION__, __LINE__, xi_context_entry_index);

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


