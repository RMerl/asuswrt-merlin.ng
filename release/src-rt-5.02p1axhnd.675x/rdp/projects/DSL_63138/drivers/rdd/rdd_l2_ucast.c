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
extern uint8_t *g_runner_tables_ptr;
extern uint32_t g_free_context_entries_number;
extern uint32_t g_free_context_entries_head;
extern uint32_t g_free_context_entries_tail;
extern uint32_t *g_free_connection_context_entries;
extern bdmf_fastlock int_lock_irq;

extern BL_LILAC_RDD_ERROR_DTE rdd_free_context_entry ( uint32_t  context_entry_index );

//#define CC_RDD_L2_DEBUG

#if defined(CC_RDD_L2_DEBUG)
extern int g_dbg_lvl;
#define __debug_l2(fmt, arg...)                             \
    if ( g_dbg_lvl > 0 )                                    \
        do {                                                \
            printk(fmt, ##arg);                             \
        } while(0)

#else
#define __debug_l2(fmt, arg...)
#endif


static inline void print_mac_addr( bdmf_mac_t *mac_addr_ptr )
{
    __debug_l2("%02X:%02X:%02X:%02X:%02X:%02X",
          mac_addr_ptr->b[0], mac_addr_ptr->b[1], mac_addr_ptr->b[2],
          mac_addr_ptr->b[3], mac_addr_ptr->b[4], mac_addr_ptr->b[5]);
}

static inline void print_buffer( char *s, uint8_t *buf_ptr, uint32_t buf_length )
{
    uint32_t i;

    __debug_l2("%s", s);

    for( i=0; i < buf_length; i++ )
        __debug_l2("%02X ", buf_ptr[i] );

    __debug_l2( "\n" );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_l2_context_entry_write ( RDD_CONTEXT_ENTRY_UNION_DTS  *xi_context_entry,
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
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_EGRESS_PHY_WRITE( xi_context_entry->fc_ucast_flow_context_entry.egress_phy, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_ANY_WRITE(xi_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DROP_WRITE(xi_context_entry->fc_ucast_flow_context_entry.drop, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_PRIORITY_WRITE(xi_context_entry->fc_ucast_flow_context_entry.priority, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_WFD_PRIO_WRITE(xi_context_entry->fc_ucast_flow_context_entry.wfd_prio, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_WFD_IDX_WRITE(xi_context_entry->fc_ucast_flow_context_entry.wfd_idx, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_NIC_WRITE(xi_context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic, xi_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_HIT_TRAP_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_hit_trap, xi_context_entry_ptr );
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

            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_WRED_HIGH_PRIO_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_wred_high_prio, xi_context_entry_ptr );
            RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_INGQOS_HIGH_PRIO_WRITE( xi_context_entry->fc_ucast_flow_context_entry.is_ingqos_high_prio, xi_context_entry_ptr );
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
    }

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_l2_context_entry_add ( RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry,
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

    context_entry->fc_ucast_flow_context_entry.valid = 1;
    context_entry_ptr = &( context_table_ptr->entry[ *context_entry_index ] );

    f_rdd_l2_context_entry_write ( context_entry, context_entry_ptr, 1 );

#if defined(CC_RDD_L2_DEBUG)
    {
        uint32_t context_entry_connection_table_index;

        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ ( context_entry_connection_table_index, context_entry_ptr );

        __debug_l2("%s, %u: connection_table_index %u, context_entry_index %u, context_entry_ptr 0x%p\n", __FUNCTION__, __LINE__, 
            context_entry_connection_table_index, *context_entry_index, context_entry_ptr);
    }
#endif

    return ( BL_LILAC_RDD_OK );
}


/* f_rdd_ipproto_lookup_port_get() gets IP Proto (TCP, UDP, etc.) and source bridge port from the protocol fields */
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_ipproto_lookup_port_get ( uint8_t    *xi_prot_ptr,
                                                                         uint8_t    *xo_lookup_port_ptr )
{
    uint8_t ipproto_idx;
    uint8_t ipproto;

    *xo_lookup_port_ptr = (*xi_prot_ptr >> TUPLE_PROTO_LOOKUP_PORT_F_OFFSET) & TUPLE_PROTO_LOOKUP_PORT_F_MASK;
    ipproto_idx = (*xi_prot_ptr >> TUPLE_PROTO_PROTOCOL_F_OFFSET) & TUPLE_PROTO_PROTOCOL_F_MASK;

    switch (ipproto_idx)
    {
        case IPPROTO_IDX_TCP:
            ipproto = IPPROTO_TCP;
            break;

        case IPPROTO_IDX_UDP:
            ipproto = IPPROTO_UDP;
            break;

        default:
            ipproto = IPPROTO_UDP;
    }

    *xi_prot_ptr = ipproto;

    return ( BL_LILAC_RDD_OK );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_l2_connection_entry_alloc ( uint32_t                 xi_hash_index,
                                                                       RDD_CONNECTION_TABLE_DTS *xi_connection_table_ptr,
                                                                       rdpa_l2_flow_key_t       *xi_l2_lookup_entry,
                                                                       uint32_t                 xi_src_mac_crc,
                                                                       uint32_t                 xi_dst_mac_crc,
                                                                       uint32_t                 *xo_tries )
{
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  tries;
    uint32_t                  connection_entry_index;
    uint32_t                  connection_entry_valid;
    uint8_t                   connection_entry_protocol;
    uint8_t                   connection_entry_tos;
    uint16_t                  connection_entry_tcp_pure_ack;
    uint32_t                  connection_entry_src_mac_crc;
    uint32_t                  connection_entry_dst_mac_crc;

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
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_PROTOCOL_READ ( connection_entry_protocol, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_TOS_READ ( connection_entry_tos, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_TCP_PURE_ACK_READ ( connection_entry_tcp_pure_ack, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_SRC_MAC_CRC_READ ( connection_entry_src_mac_crc, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_DST_MAC_CRC_READ ( connection_entry_dst_mac_crc, connection_entry_ptr );

        if ( ( connection_entry_protocol == xi_l2_lookup_entry->lookup_port) && ( connection_entry_tos == xi_l2_lookup_entry->tos ) &&
             ( connection_entry_tcp_pure_ack == xi_l2_lookup_entry->tcp_pure_ack ) &&
             ( connection_entry_src_mac_crc == xi_src_mac_crc ) &&
             ( connection_entry_dst_mac_crc == xi_dst_mac_crc ) )
        {
            return ( BL_LILAC_RDD_ERROR_LOOKUP_ENTRY_EXISTS );
        }
    }

    *xo_tries = tries;
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_l2_connection_entry_add ( BL_LILAC_RDD_ADD_CONNECTION_DTE  *xi_add_l2_connection,
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
    uint32_t                     l2_src_mac_crc;
    uint32_t                     l2_dst_mac_crc;
    BL_LILAC_RDD_ERROR_DTE       rdd_error;
    uint32_t                     bucket_overflow_counter;
    uint32_t                     entry_overflow;
    unsigned long                flags;
    uint8_t                      crc_buffer[16];
    rdpa_l2_flow_key_t           *l2_lookup_entry = (rdpa_l2_flow_key_t *) xi_add_l2_connection->l2_lookup_entry;
    uint16_t                     any_src_port_flow_counter;
    RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS *any_src_port_flow_counter_ptr;

    __debug_l2("\n%s, %u: ============================================================ \n", __FUNCTION__, __LINE__); 
    __debug_l2("%s, %u: Input Params: MACSA=", __FUNCTION__, __LINE__);
    print_mac_addr ( &l2_lookup_entry->src_mac); __debug_l2("\n");

    __debug_l2("%s, %u: Input Params: MACDA=", __FUNCTION__, __LINE__);
    print_mac_addr ( &l2_lookup_entry->dst_mac); __debug_l2("\n");

    __debug_l2("%s, %u: Input Params: vtag_num=%u eth_type=0x%04X\n", __FUNCTION__, __LINE__, 
            l2_lookup_entry->vtag_num, l2_lookup_entry->eth_type);

    __debug_l2("%s, %u: vtag0=0x%08X vtag1=0x%08X\n", __FUNCTION__, __LINE__, 
              l2_lookup_entry->vtag0, l2_lookup_entry->vtag1);

    __debug_l2("%s, %u: tos=0x%02X\n", __FUNCTION__, __LINE__, l2_lookup_entry->tos);

    if ( l2_lookup_entry->vtag_num > 2 )
    {
        __debug_l2("%s, %u: invalid params\n", __FUNCTION__, __LINE__); 
        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    if ( xi_direction == rdpa_dir_ds )
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    l2_src_mac_crc = 0;
    l2_dst_mac_crc = 0;

    entry_bytes[ 0 ] = 0;
    entry_bytes[ 1 ] = 0;
    entry_bytes[ 2 ] = 0;
    entry_bytes[ 3 ] = xi_add_l2_connection->l2_lookup_entry->lookup_port;
    entry_bytes[ 4 ] = 0;
    entry_bytes[ 5 ] = 0;
    entry_bytes[ 6 ] = xi_add_l2_connection->l2_lookup_entry->tcp_pure_ack;
    entry_bytes[ 7 ] = xi_add_l2_connection->l2_lookup_entry->tos;

    memset( crc_buffer, 0, sizeof(crc_buffer) );

    {
#if !defined(FIRMWARE_INIT)
        uint32_t vtag0 = htonl(xi_add_l2_connection->l2_lookup_entry->vtag0);
        uint32_t vtag1 = htonl(xi_add_l2_connection->l2_lookup_entry->vtag1);
        uint16_t eth_type = htons(xi_add_l2_connection->l2_lookup_entry->eth_type);

        memcpy( &crc_buffer[0], &vtag0, 4 );
        memcpy( &crc_buffer[4], &vtag1, 4 );
        memcpy( &crc_buffer[8], &xi_add_l2_connection->l2_lookup_entry->src_mac, 6 );
        memcpy( &crc_buffer[14], &eth_type, 2 );
        print_buffer( "add src_mac_crc_buffer:\n", crc_buffer, 16 );

        l2_src_mac_crc = crcbitbybit((uint8_t *)&crc_buffer[0], 16, 0, 0xffffffff, RDD_CRC_TYPE_32 );
#else
        l2_src_mac_crc = (uint32_t) &xi_add_l2_connection->l2_lookup_entry->src_mac;
#endif
        __debug_l2( "add l2_src_mac_crc: 0x%08X\n", l2_src_mac_crc );

        entry_bytes[ 8 ] = ( l2_src_mac_crc >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( l2_src_mac_crc >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( l2_src_mac_crc >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = ( l2_src_mac_crc >> 0 ) & 0xFF;

#if !defined(FIRMWARE_INIT)
        memset( crc_buffer, 0, sizeof(crc_buffer) );
        memcpy( &crc_buffer[2], &xi_add_l2_connection->l2_lookup_entry->dst_mac, 6 );
        print_buffer( "add dst_mac_crc_buffer:\n", crc_buffer, 16 );

        l2_dst_mac_crc = crcbitbybit((uint8_t *)&crc_buffer[0], 16, 0, 0xffffffff, RDD_CRC_TYPE_32 );
#else
        l2_dst_mac_crc = (uint32_t) &xi_add_l2_connection->l2_lookup_entry->dst_mac;
#endif
        __debug_l2( "add l2_dst_mac_crc: 0x%08X\n", l2_dst_mac_crc );

        entry_bytes[ 12 ] = ( l2_dst_mac_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( l2_dst_mac_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( l2_dst_mac_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( l2_dst_mac_crc >> 0 ) & 0xFF;
    }
    print_buffer( "add entry_bytes:\n", entry_bytes, 16 );

    crc_init_value = xi_add_l2_connection->l2_lookup_entry->lookup_port;

    entry_overflow = LILAC_RDD_FALSE;

    /* calculate the CRC on the connection entry */
    crc_result = crcbitbybit ( &entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    __debug_l2( "add hash_index = %u <0x%04x>\n", hash_index, hash_index ); 

    rdd_error = f_rdd_l2_connection_entry_alloc ( hash_index, connection_table_ptr, xi_add_l2_connection->l2_lookup_entry, l2_src_mac_crc, l2_dst_mac_crc, &tries );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        rdd_error = f_rdd_l2_connection_entry_alloc ( hash_index, connection_table_ptr, xi_add_l2_connection->l2_lookup_entry, l2_src_mac_crc, l2_dst_mac_crc, &tries );

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
    memset( connection_entry_ptr, 0, sizeof(RDD_CONNECTION_ENTRY_DTS) );

    xi_add_l2_connection->context_entry.fc_ucast_flow_context_entry.connection_table_index = connection_entry_index;
    xi_add_l2_connection->context_entry.fc_ucast_flow_context_entry.connection_direction = xi_direction;

    rdd_error = f_rdd_l2_context_entry_add ( &xi_add_l2_connection->context_entry, xi_direction, &context_entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    {
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_PROTOCOL_WRITE ( xi_add_l2_connection->l2_lookup_entry->lookup_port, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_TOS_WRITE ( xi_add_l2_connection->l2_lookup_entry->tos, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_TCP_PURE_ACK_WRITE ( xi_add_l2_connection->l2_lookup_entry->tcp_pure_ack, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_DST_MAC_CRC_WRITE ( l2_dst_mac_crc, connection_entry_ptr );
        RDD_FC_L2_UCAST_CONNECTION_ENTRY_SRC_MAC_CRC_WRITE ( l2_src_mac_crc, connection_entry_ptr );
    }

    if (xi_add_l2_connection->l2_lookup_entry->lookup_port == BL_LILAC_RDD_ANY_BRIDGE_PORT)
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

    RDD_CONNECTION_ENTRY_COMMAND_LIST_LENGTH_64_WRITE( xi_add_l2_connection->context_entry.fc_ucast_flow_context_entry.command_list_length_64, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_KEY_EXTEND_WRITE ( 0, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_WRITE ( context_entry_index, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_VALID_WRITE ( LILAC_RDD_ON, connection_entry_ptr );

    /* return the index of the entry in the table */
    xi_add_l2_connection->xo_entry_index = context_entry_index;
    __debug_l2("%s, %u: connection_entry_ptr 0x%p\n", __FUNCTION__, __LINE__, connection_entry_ptr);

    if ( entry_overflow == LILAC_RDD_TRUE )
    {
        /* set entry_overflow in the context of the entry */
        context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
        context_entry_ptr = &(context_table_ptr->entry[ context_entry_index ] );

        RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_OVERFLOW_WRITE ( LILAC_RDD_TRUE, context_entry_ptr );
    }

#if defined(CC_RDD_L2_DEBUG)
    {
        context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
        context_entry_ptr = &(context_table_ptr->entry[ context_entry_index ] );

        __debug_l2("%s, %u: connection_table_index %u\n", __FUNCTION__, __LINE__, context_entry_ptr->fc_ucast_flow_context_entry.connection_table_index);
    }
#endif

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_l2_connection_entry_lookup ( uint32_t               xi_hash_index,
                                                                     RDD_CONNECTION_TABLE_DTS  *xi_connection_table_ptr,
                                                                     rdpa_l2_flow_key_t        *xi_l2_lookup_entry,
                                                                     uint32_t                  xi_src_mac_crc,
                                                                     uint32_t                  xi_dst_mac_crc,
                                                                     uint32_t                  *xo_tries )
{
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  tries;
    uint32_t                  connection_entry_index;
    uint32_t                  connection_entry_valid;
    uint8_t                   connection_entry_protocol;
    uint8_t                   connection_entry_tos;
    uint8_t                   connection_entry_tcp_pure_ack;
    uint32_t                  connection_entry_src_mac_crc;
    uint32_t                  connection_entry_dst_mac_crc;
 
    for ( tries = 0; tries < LILAC_RDD_CONNECTION_TABLE_SET_SIZE; tries++ )
    {
        connection_entry_index = xi_hash_index + tries;

        connection_entry_ptr = &( xi_connection_table_ptr->entry[ connection_entry_index ] );

        RDD_CONNECTION_ENTRY_VALID_READ ( connection_entry_valid, connection_entry_ptr );

        if ( connection_entry_valid )
        {
            RDD_FC_L2_UCAST_CONNECTION_ENTRY_PROTOCOL_READ ( connection_entry_protocol, connection_entry_ptr );
            RDD_FC_L2_UCAST_CONNECTION_ENTRY_TOS_READ ( connection_entry_tos, connection_entry_ptr );
            RDD_FC_L2_UCAST_CONNECTION_ENTRY_TCP_PURE_ACK_READ ( connection_entry_tcp_pure_ack, connection_entry_ptr );
            RDD_FC_L2_UCAST_CONNECTION_ENTRY_SRC_MAC_CRC_READ ( connection_entry_src_mac_crc, connection_entry_ptr );
            RDD_FC_L2_UCAST_CONNECTION_ENTRY_DST_MAC_CRC_READ ( connection_entry_dst_mac_crc, connection_entry_ptr );

            if ( ( connection_entry_protocol == xi_l2_lookup_entry->lookup_port) && ( connection_entry_tos == xi_l2_lookup_entry->tos ) &&
                 ( connection_entry_tcp_pure_ack == xi_l2_lookup_entry->tcp_pure_ack ) &&
                 ( connection_entry_src_mac_crc == xi_src_mac_crc ) &&
                 ( connection_entry_dst_mac_crc == xi_dst_mac_crc ) )
            {
                break;
            }
        }
    }

    *xo_tries = tries;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_l2_connection_entry_delete ( bdmf_index  xi_entry_index )
{
    RDD_CONNECTION_TABLE_DTS     *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS     *connection_entry_ptr;
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint32_t                     connection_entry_valid;
    uint32_t                     connection_entry_context_table_index;
    uint8_t                      connection_entry_protocol;
    uint8_t                      connection_entry_lookup_port;
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

        f_rdd_ipproto_lookup_port_get ( &connection_entry_protocol, &connection_entry_lookup_port );

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

        rdd_free_context_entry ( xi_entry_index );

        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_OK );
    }
    else
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }
}


BL_LILAC_RDD_ERROR_DTE rdd_l2_connection_entry_search ( BL_LILAC_RDD_ADD_CONNECTION_DTE  *xi_get_connection,
                                                        rdpa_traffic_dir                 xi_direction,
                                                        bdmf_index                       *xo_entry_index )
{
    RDD_CONNECTION_TABLE_DTS  *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint8_t                   entry_bytes[ LILAC_RDD_CONNECTION_ENTRY_SIZE ];
    uint32_t                  crc_init_value, crc_result, hash_index, tries;
    uint32_t                  connection_entry_index;
    uint16_t                  connection_entry_context_index;
    uint32_t                  l2_src_mac_crc;
    uint32_t                  l2_dst_mac_crc;
    unsigned long             flags;
    uint8_t                   crc_buffer[16];
    rdpa_l2_flow_key_t           *l2_lookup_entry = (rdpa_l2_flow_key_t *) xi_get_connection->l2_lookup_entry;

    __debug_l2("\n%s, %u: ============================================================ \n", __FUNCTION__, __LINE__); 
    __debug_l2("%s, %u: Input Params: MACSA=", __FUNCTION__, __LINE__);
    print_mac_addr ( &l2_lookup_entry->src_mac); __debug_l2("\n");

    __debug_l2("%s, %u: Input Params: MACDA=", __FUNCTION__, __LINE__);
    print_mac_addr ( &l2_lookup_entry->dst_mac); __debug_l2("\n");

    __debug_l2("%s, %u: Input Params: vtag_num=%u eth_type=0x%04X\n", __FUNCTION__, __LINE__, 
            l2_lookup_entry->vtag_num, l2_lookup_entry->eth_type);

    __debug_l2("%s, %u: vtag0=0x%08X vtag1=0x%08X\n", __FUNCTION__, __LINE__, 
              l2_lookup_entry->vtag0, l2_lookup_entry->vtag1);

    if ( l2_lookup_entry->vtag_num > 2 )
    {
        __debug_l2("%s, %u: invalid params\n", __FUNCTION__, __LINE__); 
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    if ( xi_direction == rdpa_dir_ds )
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    l2_src_mac_crc = 0;
    l2_dst_mac_crc = 0;

    entry_bytes[ 0 ] = 0;
    entry_bytes[ 1 ] = 0;
    entry_bytes[ 2 ] = 0;
    entry_bytes[ 3 ] = xi_get_connection->l2_lookup_entry->lookup_port;
    entry_bytes[ 4 ] = 0; 
    entry_bytes[ 5 ] = 0;
    entry_bytes[ 6 ] = 0;
    entry_bytes[ 7 ] = 0;

    memset( &crc_buffer[0], 0, sizeof(crc_buffer) );

    {
#if !defined(FIRMWARE_INIT)
        uint32_t vtag0 = htonl(xi_get_connection->l2_lookup_entry->vtag0);
        uint32_t vtag1 = htonl(xi_get_connection->l2_lookup_entry->vtag1);
        uint16_t eth_type = htons(xi_get_connection->l2_lookup_entry->eth_type);

        memcpy( &crc_buffer[0], &vtag0, 4 );
        memcpy( &crc_buffer[4], &vtag1, 4 );
        memcpy( &crc_buffer[8], &xi_get_connection->l2_lookup_entry->src_mac, 6 );
        memcpy( &crc_buffer[14], &eth_type, 2 );
        print_buffer( "search src_mac_crc_buffer:\n", crc_buffer, 16 );

        l2_src_mac_crc = crcbitbybit((uint8_t *)&crc_buffer[0], 16, 0, 0xffffffff, RDD_CRC_TYPE_32 );
#else
        l2_src_mac_crc = (uint32_t) &xi_get_connection->l2_lookup_entry->src_mac.b[0];
#endif
        __debug_l2( "search l2_src_mac_crc: 0x%08X\n", l2_src_mac_crc );

        entry_bytes[ 8 ] = ( l2_src_mac_crc >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( l2_src_mac_crc >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( l2_src_mac_crc >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = ( l2_src_mac_crc >> 0 ) & 0xFF;

#if !defined(FIRMWARE_INIT)
        memset( &crc_buffer[0], 0, sizeof(crc_buffer) );
        memcpy( &crc_buffer[2], &xi_get_connection->l2_lookup_entry->dst_mac.b[0], 6 );
        print_buffer( "search dst_mac_crc_buffer:\n", crc_buffer, 16 );

        l2_dst_mac_crc = crcbitbybit((uint8_t *)&crc_buffer[0], 16, 0, 0xffffffff, RDD_CRC_TYPE_32 );
#else
        l2_dst_mac_crc = (uint32_t) &xi_get_connection->l2_lookup_entry->dst_mac.b[0];
#endif
        __debug_l2( "search l2_dst_mac_crc: 0x%08X\n", l2_dst_mac_crc );
        entry_bytes[ 12 ] = ( l2_dst_mac_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( l2_dst_mac_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( l2_dst_mac_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( l2_dst_mac_crc >> 0 ) & 0xFF;
    }
    print_buffer( "search entry_bytes:\n", entry_bytes, 16 );

    crc_init_value = xi_get_connection->l2_lookup_entry->lookup_port;

    /* calculate the CRC on the connection entry */
    crc_result = crcbitbybit ( &entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_CONNECTION_TABLE_SIZE / LILAC_RDD_CONNECTION_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_CONNECTION_TABLE_SET_SIZE;

    f_rdd_l2_connection_entry_lookup ( hash_index, connection_table_ptr, xi_get_connection->l2_lookup_entry, l2_src_mac_crc, l2_dst_mac_crc, &tries );

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        hash_index = ( hash_index + LILAC_RDD_CONNECTION_TABLE_SET_SIZE ) & ( RDD_CONNECTION_TABLE_SIZE - 1 );

        f_rdd_l2_connection_entry_lookup ( hash_index, connection_table_ptr, xi_get_connection->l2_lookup_entry, l2_src_mac_crc, l2_dst_mac_crc, &tries );

        if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
        {
            bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
        }
    }

    __debug_l2( "search hash_index = %u <0x%04x>\n", hash_index, hash_index ); 
    connection_entry_index = hash_index + tries;

    connection_entry_ptr = &( connection_table_ptr->entry[ connection_entry_index ] );

    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( connection_entry_context_index, connection_entry_ptr );
    *xo_entry_index = connection_entry_context_index;

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_l2_connection_entry_get ( rdpa_traffic_dir    xi_direction,
                                                     uint32_t            xi_entry_index,
                                                     rdpa_l2_flow_key_t  *xo_connection_entry,
                                                     bdmf_index          *xo_context_index )
{
    RDD_CONNECTION_TABLE_DTS  *connection_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *connection_entry_ptr;
    uint32_t                  connection_entry_valid;
    uint32_t                  connection_entry_src_mac_crc;
    uint32_t                  connection_entry_dst_mac_crc;
    uint8_t                   connection_entry_protocol;
    uint8_t                   connection_entry_tos;
    uint8_t                   connection_entry_tcp_pure_ack;


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

    RDD_FC_L2_UCAST_CONNECTION_ENTRY_PROTOCOL_READ ( connection_entry_protocol, connection_entry_ptr );
    RDD_FC_L2_UCAST_CONNECTION_ENTRY_TOS_READ ( connection_entry_tos, connection_entry_ptr );
    RDD_FC_L2_UCAST_CONNECTION_ENTRY_TCP_PURE_ACK_READ ( connection_entry_tcp_pure_ack, connection_entry_ptr );
    RDD_FC_L2_UCAST_CONNECTION_ENTRY_SRC_MAC_CRC_READ ( connection_entry_src_mac_crc, connection_entry_ptr );
    RDD_FC_L2_UCAST_CONNECTION_ENTRY_DST_MAC_CRC_READ ( connection_entry_dst_mac_crc, connection_entry_ptr );
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( *xo_context_index, connection_entry_ptr );

    xo_connection_entry->lookup_port = connection_entry_protocol;
    xo_connection_entry->tos = connection_entry_tos;
    xo_connection_entry->tcp_pure_ack = connection_entry_tcp_pure_ack;
    connection_entry_src_mac_crc = ntohl(connection_entry_src_mac_crc );
    connection_entry_dst_mac_crc = ntohl(connection_entry_dst_mac_crc );
    memcpy( &xo_connection_entry->src_mac, &connection_entry_src_mac_crc, 4 );
    memcpy( &xo_connection_entry->dst_mac, &connection_entry_dst_mac_crc, 4 );

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_l2_context_entry_get ( bdmf_index                   xi_entry_index,
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
        return ( BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID );

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
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_EGRESS_PHY_READ( xo_context_entry->fc_ucast_flow_context_entry.egress_phy, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_WRED_HIGH_PRIO_READ( xo_context_entry->fc_ucast_flow_context_entry.is_wred_high_prio, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_INGQOS_HIGH_PRIO_READ( xo_context_entry->fc_ucast_flow_context_entry.is_ingqos_high_prio, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MTU_READ( xo_context_entry->fc_ucast_flow_context_entry.mtu, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_READ( xo_context_entry->fc_ucast_flow_context_entry.is_tos_mangle, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_TOS_READ( xo_context_entry->fc_ucast_flow_context_entry.tos, context_entry_ptr );
                RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_SERVICE_QUEUE_ID_READ( xo_context_entry->fc_ucast_flow_context_entry.service_queue_id, context_entry_ptr );
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

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_l2_context_entry_flwstat_get ( bdmf_index                   xi_entry_index,
                                                          rdd_fc_context_t            *xo_context_entry )
{
    RDD_CONTEXT_TABLE_DTS        *context_table_ptr;
    RDD_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    uint32_t valid;

    if ( xi_entry_index >= RDD_CONTEXT_TABLE_SIZE )
    {
        return ( BDMF_ERR_PARM );
    }

    context_table_ptr = ( RDD_CONTEXT_TABLE_DTS * )ContextTableBase;
    context_entry_ptr = &(context_table_ptr->entry[ xi_entry_index ] );
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_VALID_READ( valid, context_entry_ptr );

    if(!valid)
    {
        return BDMF_ERR_NOENT;
    }

    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_READ( xo_context_entry->fc_ucast_flow_context_entry.multicast_flag, context_entry_ptr );
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_IS_L2_ACCEL_READ( xo_context_entry->fc_ucast_flow_context_entry.is_l2_accel, context_entry_ptr );
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_READ( xo_context_entry->fc_ucast_flow_context_entry.flow_hits, context_entry_ptr );
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_READ( xo_context_entry->fc_ucast_flow_context_entry.flow_bytes, context_entry_ptr );

    return BDMF_ERR_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_l2_context_entry_modify ( RDD_CONTEXT_ENTRY_UNION_DTS  *xi_context_entry,
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
    context_entry_ptr = &(context_table_ptr->entry[ xi_entry_index ] );

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );
    f_rdd_l2_context_entry_write ( xi_context_entry, context_entry_ptr, 0 );
    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );

    return ( BL_LILAC_RDD_OK );
}

