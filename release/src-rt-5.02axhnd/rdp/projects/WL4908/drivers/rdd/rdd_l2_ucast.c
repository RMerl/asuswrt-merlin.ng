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

#include "rdp_natcache.h"
//#include "bdmf_system_common.h"

#define RDD_NAT_CACHE_L2_LKP_ENTRY_SIZE sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS)

/* Address tables for indexed DPE blocks */
enum {
    natc_nop = 0,
    natc_lookup,
    natc_add,
    natc_del,
};

#define NATC_STATUS_BUSY_BIT  (1 << 4)
#define NATC_STATUS_ERROR_BIT (1 << 5)
#define NATC_STATUS_MISS_BIT  (1 << 6)
#define TIME_OUT_MS                  20
#define KEY_LEN_SHIFT                8
#define NAT_CACHE_SEARCH_ENGINES_NUM 4

/* Offsets must correspond to current rdd_data_structures_auto.h number_of_ports and port_mask offsets. */
#define RDD_FC_NATC_MCAST_FLOW_CONTEXT_ENTRY_NUM_PORTS_PORT_MASK_WRITE( v, p ) FIELD_MWRITE_16((uint8_t *)p + 8, 0, 12, v )


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

extern uint8_t  *ContextTableBase;
extern uint8_t  *ContextContTableBase;
extern uint8_t  *NatCacheTableBase;

extern uint32_t  g_free_context_entries_number;
extern uint32_t  g_free_flow_entries_number;
extern uint32_t  g_free_flow_entries_head;
extern uint32_t  *g_free_flow_entries;

extern bdmf_fastlock int_lock_irq;

extern volatile uint32_t result_regs_addr[NAT_CACHE_SEARCH_ENGINES_NUM];
extern volatile uint32_t status_regs_addr[NAT_CACHE_SEARCH_ENGINES_NUM];
extern volatile uint32_t hit_count_reg_addr[NAT_CACHE_SEARCH_ENGINES_NUM];
extern volatile uint32_t byte_count_reg_addr[NAT_CACHE_SEARCH_ENGINES_NUM];

//#define CC_RDD_L2_DEBUG

#if defined(CC_RDD_L2_DEBUG)
#define __debug_l2(fmt, arg...)                             \
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



static int f_rdd_l2_context_entry_write ( rdd_fc_context_t            *context_entry,
                                          uint32_t                     nat_cache_entry_index,
                                          uint32_t                     is_new_entry )
{
    uint32_t i;
    RDD_NATC_CONTEXT_TABLE_DTS   *natc_context_table_ptr;
    RDD_NATC_CONTEXT_ENTRY_UNION_DTS *natc_context_entry_ptr;
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr;
    RDD_CONTEXT_CONTINUATION_ENTRY_DTS *context_cont_entry_ptr;
    int continuation_flag;

    natc_context_table_ptr = ( RDD_NATC_CONTEXT_TABLE_DTS * )ContextTableBase;
    natc_context_entry_ptr = &( natc_context_table_ptr->entry[ nat_cache_entry_index ] );
    context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS * )ContextContTableBase;
    context_cont_entry_ptr = &( context_cont_table_ptr->entry[ nat_cache_entry_index ] );

#if defined(CC_RDD_L2_DEBUG)
    {
        __debug_l2("%s, %u: DDR ctxt entry: nat_cache_entry_index %u nat_cache_entry_ptr 0x%p,\n", __FUNCTION__, __LINE__, nat_cache_entry_index, natc_context_entry_ptr );
        __debug_l2("%s, %u: DDR ctxt cont entry: context_cont_entry_ptr 0x%p,\n", __FUNCTION__, __LINE__, context_cont_entry_ptr );
    }
#endif

    if(is_new_entry)
    {
        /* + 4 because remaining fields of context can be used for command list if continuation not required */
        continuation_flag= ( (context_entry->fc_ucast_flow_context_entry.command_list_length_64*8 > RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER + 4) || context_entry->fc_ucast_flow_context_entry.drop ) ? 1 : 0;

        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_WRITE( 0, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_WRITE( 0, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_WRITE( 0, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_CONTEXT_CONTINUATION_FLAG_WRITE ( continuation_flag, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_ROUTED_WRITE( context_entry->fc_ucast_flow_context_entry.is_routed, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_L2_ACCEL_WRITE( context_entry->fc_ucast_flow_context_entry.is_l2_accel, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_MTU_WRITE( context_entry->fc_ucast_flow_context_entry.mtu, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_WRITE( context_entry->fc_ucast_flow_context_entry.is_tos_mangle, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_TOS_WRITE( context_entry->fc_ucast_flow_context_entry.tos, natc_context_entry_ptr );


        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_EGRESS_PHY_WRITE( context_entry->fc_ucast_flow_context_entry.egress_phy, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_ANY_WRITE(context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any, natc_context_entry_ptr);
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_DROP_WRITE(context_entry->fc_ucast_flow_context_entry.drop, natc_context_entry_ptr);
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_PRIORITY_WRITE(context_entry->fc_ucast_flow_context_entry.priority, natc_context_entry_ptr);
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_WFD_PRIO_WRITE(context_entry->fc_ucast_flow_context_entry.wfd_prio, natc_context_entry_ptr);
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_WFD_IDX_WRITE(context_entry->fc_ucast_flow_context_entry.wfd_idx, natc_context_entry_ptr);
        RDD_CONTEXT_CONTINUATION_ENTRY_IS_UNICAST_WFD_NIC_WRITE(context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic, context_cont_entry_ptr);

        if (context_entry->fc_ucast_flow_context_entry.egress_phy == rdd_egress_phy_wlan)
        {
            if (context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any)
            {
                if (context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic)
                {
                    RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_CHAIN_IDX_WRITE(context_entry->fc_ucast_flow_context_wfd_nic_entry.chain_idx, natc_context_entry_ptr);
                }
                else
                {
                    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_WIFI_SSID_WRITE(context_entry->fc_ucast_flow_context_wfd_dhd_entry.wifi_ssid, natc_context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_FLOW_RING_ID_WRITE(context_entry->fc_ucast_flow_context_wfd_dhd_entry.flow_ring_id, natc_context_entry_ptr);
                }
            }
#if defined(CONFIG_DHD_RUNNER)
            else
            {
                RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_RADIO_IDX_WRITE(context_entry->fc_ucast_flow_context_rnr_dhd_entry.radio_idx, natc_context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_WIFI_SSID_WRITE(context_entry->fc_ucast_flow_context_rnr_dhd_entry.wifi_ssid, natc_context_entry_ptr);
                RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_FLOW_RING_ID_WRITE(context_entry->fc_ucast_flow_context_rnr_dhd_entry.flow_ring_id, natc_context_entry_ptr);
            }
#endif
        }
        else
        {
            RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_EGRESS_PORT_WRITE(context_entry->fc_ucast_flow_context_eth_xtm_entry.egress_port, natc_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_TRAFFIC_CLASS_WRITE(context_entry->fc_ucast_flow_context_eth_xtm_entry.traffic_class, natc_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_RATE_CONTROLLER_WRITE(context_entry->fc_ucast_flow_context_eth_xtm_entry.rate_controller, natc_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_LAG_PORT_WRITE(context_entry->fc_ucast_flow_context_eth_xtm_entry.egress_info , natc_context_entry_ptr);
        }
        RDD_CONTEXT_CONTINUATION_ENTRY_CONNECTION_DIRECTION_WRITE ( context_entry->fc_ucast_flow_context_entry.connection_direction, context_cont_entry_ptr );
        RDD_CONTEXT_CONTINUATION_ENTRY_CONNECTION_TABLE_INDEX_WRITE ( context_entry->fc_ucast_flow_context_entry.connection_table_index, context_cont_entry_ptr );

        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IP_ADDRESSES_TABLE_INDEX_WRITE( context_entry->fc_ucast_flow_context_entry.ip_addresses_table_index, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_PATHSTAT_IDX_WRITE( context_entry->fc_ucast_flow_context_entry.pathstat_idx, natc_context_entry_ptr );
        
        if (continuation_flag)
        {
            int remaining_length;

            if (!context_entry->fc_ucast_flow_context_entry.drop)
            {
                remaining_length = context_entry->fc_ucast_flow_context_entry.command_list_length_64*8 - RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER;
            }
            else
            {
                remaining_length = RDD_CONTEXT_CONTINUATION_ENTRY_COMMAND_LIST_NUMBER;
            }

            for ( i = 0; i < RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER; i++ )
            {
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_WRITE ( context_entry->fc_ucast_flow_context_entry.command_list[ i ], 
                                                                          natc_context_entry_ptr, i );
            }

            for ( i = 0; i < remaining_length; i++ )
            {
                RDD_CONTEXT_CONTINUATION_ENTRY_COMMAND_LIST_WRITE( context_entry->fc_ucast_flow_context_entry.command_list[ i + RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER ], 
                                                                   context_cont_entry_ptr, i );
            }

            RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_REMAINING_LENGTH_WRITE ( remaining_length, natc_context_entry_ptr ); 
            RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_CONTEXT_CONTINUATION_TABLE_INDEX_WRITE ( nat_cache_entry_index, natc_context_entry_ptr );    
        }
        else
        {
            /* + 4 because remaining fields of context can be used for command list if continuation not required */
            for ( i = 0; i < RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER + 4; i++ )
            {
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_WRITE ( context_entry->fc_ucast_flow_context_entry.command_list[ i ], 
                                                                          natc_context_entry_ptr, i );
            }
        }

        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_HIGH_PRIO_WRITE( context_entry->fc_ucast_flow_context_entry.is_high_prio, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_MTU_WRITE( context_entry->fc_ucast_flow_context_entry.mtu, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_WRITE( context_entry->fc_ucast_flow_context_entry.is_tos_mangle, natc_context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_TOS_WRITE( context_entry->fc_ucast_flow_context_entry.tos, natc_context_entry_ptr );
        if (context_entry->fc_ucast_flow_context_entry.egress_phy != rdd_egress_phy_wlan)
        {
            RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_TRAFFIC_CLASS_WRITE(context_entry->fc_ucast_flow_context_eth_xtm_entry.traffic_class, natc_context_entry_ptr);
            RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_RATE_CONTROLLER_WRITE(context_entry->fc_ucast_flow_context_eth_xtm_entry.rate_controller, natc_context_entry_ptr);
        }

        RDD_CONTEXT_CONTINUATION_ENTRY_VALID_WRITE ( context_entry->fc_ucast_flow_context_entry.valid, context_cont_entry_ptr );
    }

    RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_SERVICE_QUEUE_ID_WRITE( context_entry->fc_ucast_flow_context_entry.service_queue_id, natc_context_entry_ptr );

    return BDMF_ERR_OK;
}

static int f_rdd_l2_context_entry_add ( rdd_fc_context_t            *context_entry,
                                        rdpa_traffic_dir            direction,
                                        uint32_t                    nat_cache_entry_index,
                                        uint32_t                    *flow_entry_index )
{
    assert(g_free_context_entries_number > 0);

    if (g_free_context_entries_number > RDD_RESERVED_CONTEXT_ENTRIES)
    {
        int i;

        for (i = 0; i < RDD_FLOW_ENTRIES_SIZE; i++)
        {
            if (!(g_free_flow_entries[g_free_flow_entries_head] & RDD_FLOW_ENTRY_VALID))
            {
                *flow_entry_index = g_free_flow_entries_head;
                __debug_l2("%s, %u: DDR &g_free_flow_entries[%u]=0x%p added nat_cache_entry_index=%u\n", 
                        __FUNCTION__, __LINE__, g_free_flow_entries_head, &g_free_flow_entries[g_free_flow_entries_head], nat_cache_entry_index);
                g_free_flow_entries[g_free_flow_entries_head] = RDD_FLOW_ENTRY_VALID | nat_cache_entry_index;
                g_free_flow_entries_head = (g_free_flow_entries_head + 1) % RDD_FLOW_ENTRIES_SIZE;
                g_free_flow_entries_number--;
                g_free_context_entries_number--;
                break;
            }
            g_free_flow_entries_head = (g_free_flow_entries_head + 1) % RDD_FLOW_ENTRIES_SIZE;
        }

        if (i == RDD_FLOW_ENTRIES_SIZE)
        {
            return BDMF_ERR_NORES;
        }
    }
    else
    {
        return BDMF_ERR_NORES;
    }

    context_entry->fc_mcast_flow_context_entry.valid = 1;

    f_rdd_l2_context_entry_write ( context_entry, nat_cache_entry_index, 1 /*is_new_entry*/ );

    return BDMF_ERR_OK;
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

static inline int f_rdd_l2_nat_cache_lkp_entry_alloc ( uint32_t                  hash_index,
                                                       rdpa_l2_flow_key_t        *lookup_entry,
                                                       uint32_t                  l2_src_mac_crc,
                                                       uint32_t                  l2_dst_mac_crc,
                                                       uint32_t                  *tries_res )
{
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS  *nat_cache_lkp_entry_ptr;
    uint32_t                  tries;
    uint32_t                  nat_cache_entry_index;
    uint32_t                  nat_cache_lkp_entry_valid;
    uint8_t                   nat_cache_lkp_entry_protocol;
    uint16_t                  nat_cache_lkp_entry_tos;
    uint16_t                  nat_cache_lkp_entry_tcp_pure_ack;
    uint32_t                  nat_cache_lkp_entry_src_mac_crc;
    uint32_t                  nat_cache_lkp_entry_dst_mac_crc;
    RDD_NAT_CACHE_TABLE_DTS   *nat_cache_table_ptr = (RDD_NAT_CACHE_TABLE_DTS *)NatCacheTableBase;

    for ( tries = 0; tries < RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE; tries++ )
    {
        nat_cache_entry_index = hash_index + tries;

        nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &( nat_cache_table_ptr->entry[ nat_cache_entry_index ] );

        RDD_NAT_CACHE_L2_LKP_ENTRY_VALID_READ ( nat_cache_lkp_entry_valid, nat_cache_lkp_entry_ptr );

        if ( !( nat_cache_lkp_entry_valid ) )
        {
            break;
        }

        /* if entry is valid, check if it matches entry being added */
        RDD_NAT_CACHE_L2_LKP_ENTRY_PROTOCOL_READ ( nat_cache_lkp_entry_protocol, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_TOS_READ ( nat_cache_lkp_entry_tos, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_TCP_PURE_ACK_READ ( nat_cache_lkp_entry_tcp_pure_ack, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_SRC_MAC_CRC_READ ( nat_cache_lkp_entry_src_mac_crc, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_DST_MAC_CRC_READ ( nat_cache_lkp_entry_dst_mac_crc, nat_cache_lkp_entry_ptr );
 
        /* nat_cache_lkp_entry_protocol value is lookup_port because prot=0 for L2 flows */
        if ( ( nat_cache_lkp_entry_protocol == lookup_entry->lookup_port ) &&
             ( nat_cache_lkp_entry_tos == lookup_entry->tos ) &&
             ( nat_cache_lkp_entry_tcp_pure_ack == lookup_entry->tcp_pure_ack ) &&
             ( nat_cache_lkp_entry_src_mac_crc == l2_src_mac_crc ) &&
             ( nat_cache_lkp_entry_dst_mac_crc == l2_dst_mac_crc ) )
        {
            return BDMF_ERR_ALREADY;
        }
    }

    *tries_res = tries;
    return BDMF_ERR_OK;
}

static void rdd_connection_hash_function(uint32_t *hash_idx, uint8_t *connection_entry)
{
    *hash_idx = crcbitbybit(connection_entry, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS), 0, 0, RDD_CRC_TYPE_32);
    *hash_idx = (*hash_idx >> 16) ^ (*hash_idx & 0xffff);
}

void rdd_l2_connection_entry_hash( rdpa_l2_flow_key_t    *l2_lookup_entry,
                                  uint32_t              *hash_index_ptr,
                                  uint32_t              *l2_src_mac_crc_ptr,
                                  uint32_t              *l2_dst_mac_crc_ptr )
{
    uint8_t                      entry_bytes[ RDD_NAT_CACHE_L2_LKP_ENTRY_SIZE ];
    uint32_t                     l2_src_mac_crc;
    uint32_t                     l2_dst_mac_crc;
    uint8_t                      crc_buffer[16];

    l2_src_mac_crc = 0;
    l2_dst_mac_crc = 0;

    entry_bytes[ 0 ] = (1 << 7); /* valid bit */
    entry_bytes[ 1 ] = 0;
    entry_bytes[ 2 ] = 0;
    entry_bytes[ 3 ] = l2_lookup_entry->lookup_port;    /* protocol value for L2 flow */
    entry_bytes[ 4 ] = 0;
    entry_bytes[ 5 ] = 0;
    entry_bytes[ 6 ] = l2_lookup_entry->tcp_pure_ack;
    entry_bytes[ 7 ] = l2_lookup_entry->tos;

    memset( crc_buffer, 0, sizeof(crc_buffer) );

    {
#if !defined(FIRMWARE_INIT)
        uint32_t vtag0 = htonl(l2_lookup_entry->vtag0);
        uint32_t vtag1 = htonl(l2_lookup_entry->vtag1);
        uint16_t eth_type = htons(l2_lookup_entry->eth_type);

        memcpy( &crc_buffer[0], &vtag0, 4 );
        memcpy( &crc_buffer[4], &vtag1, 4 );
        memcpy( &crc_buffer[8], &l2_lookup_entry->src_mac, 6 );
        memcpy( &crc_buffer[14], &eth_type, 2 );
        print_buffer( "add src_mac_crc_buffer:\n", crc_buffer, 16 );

        l2_src_mac_crc = crcbitbybit((uint8_t *)&crc_buffer[0], 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
#else
        l2_src_mac_crc = (uint32_t) &l2_lookup_entry->src_mac;
#endif
        __debug_l2( "add l2_src_mac_crc: 0x%08X\n", l2_src_mac_crc );

        entry_bytes[ 8 ] = ( l2_src_mac_crc >> 24 ) & 0xFF;
        entry_bytes[ 9 ] = ( l2_src_mac_crc >> 16 ) & 0xFF;
        entry_bytes[ 10 ] = ( l2_src_mac_crc >> 8 ) & 0xFF;
        entry_bytes[ 11 ] = ( l2_src_mac_crc >> 0 ) & 0xFF;

#if !defined(FIRMWARE_INIT)
        memset( crc_buffer, 0, sizeof(crc_buffer) );
        memcpy( &crc_buffer[2], &l2_lookup_entry->dst_mac, 6 );
        print_buffer( "add dst_mac_crc_buffer:\n", crc_buffer, 16 );

        l2_dst_mac_crc = crcbitbybit((uint8_t *)&crc_buffer[0], 16, 0, 0xffffffff, RDD_CRC_TYPE_32);
#else
        l2_dst_mac_crc = (uint32_t) &l2_lookup_entry->dst_mac;
#endif
        __debug_l2( "add l2_dst_mac_crc: 0x%08X\n", l2_dst_mac_crc );

        entry_bytes[ 12 ] = ( l2_dst_mac_crc >> 24 ) & 0xFF;
        entry_bytes[ 13 ] = ( l2_dst_mac_crc >> 16 ) & 0xFF;
        entry_bytes[ 14 ] = ( l2_dst_mac_crc >> 8 ) & 0xFF;
        entry_bytes[ 15 ] = ( l2_dst_mac_crc >> 0 ) & 0xFF;
    }
    print_buffer( "add entry_bytes:\n", entry_bytes, 16 );

    rdd_connection_hash_function(hash_index_ptr, entry_bytes);
    *l2_src_mac_crc_ptr = l2_src_mac_crc;
    *l2_dst_mac_crc_ptr = l2_dst_mac_crc;
}

int rdd_l2_connection_entry_add ( rdd_l2_flow_t  *add_connection,
                                  rdpa_traffic_dir                 direction )
{
    RDD_NAT_CACHE_TABLE_DTS      *nat_cache_table_ptr = (RDD_NAT_CACHE_TABLE_DTS *)NatCacheTableBase;
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS  *nat_cache_lkp_entry_ptr;
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr;
    RDD_CONTEXT_CONTINUATION_ENTRY_DTS *context_cont_entry_ptr;
    uint32_t                     hash_index, tries = 0;
    uint32_t                     nat_cache_entry_index;
    uint32_t                     flow_entry_index;
    uint32_t                     l2_src_mac_crc;
    uint32_t                     l2_dst_mac_crc;
    int                          bdmf_error;
    unsigned long                flags;
    int                          create_dup_key=0; /*NAT Cache errata workaround*/
    rdpa_l2_flow_key_t           *l2_lookup_entry = (rdpa_l2_flow_key_t *) add_connection->l2_lookup_entry;
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
        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    rdd_l2_connection_entry_hash( l2_lookup_entry, &hash_index, &l2_src_mac_crc, &l2_dst_mac_crc );

    __debug_l2( "add hash_index = %u <0x%04x>\n", hash_index, hash_index ); 

    bdmf_error = f_rdd_l2_nat_cache_lkp_entry_alloc ( hash_index, add_connection->l2_lookup_entry, l2_src_mac_crc, l2_dst_mac_crc, &tries );

    if ( bdmf_error != BDMF_ERR_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        __debug_l2("%s, %u: error =0x%x\n", __FUNCTION__, __LINE__, bdmf_error);
        return ( bdmf_error );
    }

    if ( tries == RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return BDMF_ERR_IGNORE;
    }

    /*Wrap index at 64K - Part of NAT Cache workaround below*/
    nat_cache_entry_index = (hash_index + tries) & (RDD_NAT_CACHE_TABLE_SIZE - 1);

    nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &( nat_cache_table_ptr->entry[ nat_cache_entry_index ] );
    memset( nat_cache_lkp_entry_ptr, 0, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS) );

    if( add_connection->context_entry.fc_ucast_flow_context_entry.multicast_flag == 0 )
    {
        add_connection->context_entry.fc_ucast_flow_context_entry.connection_table_index = nat_cache_entry_index;
        add_connection->context_entry.fc_ucast_flow_context_entry.connection_direction = direction;
    }

    bdmf_error = f_rdd_l2_context_entry_add ( &add_connection->context_entry, direction, nat_cache_entry_index, &flow_entry_index );
    if (bdmf_error != BDMF_ERR_OK)
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return BDMF_ERR_NORES;
    }

    if (add_connection->l2_lookup_entry->lookup_port == BL_LILAC_RDD_ANY_BRIDGE_PORT)
    {
        any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 
   
        MREAD_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
        any_src_port_flow_counter++;
        MWRITE_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );

        any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 

        MREAD_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
        any_src_port_flow_counter++;
        MWRITE_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
    }

    do
    {
        RDD_NAT_CACHE_L2_LKP_ENTRY_PROTOCOL_WRITE ( add_connection->l2_lookup_entry->lookup_port, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_TOS_WRITE ( add_connection->l2_lookup_entry->tos, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_TCP_PURE_ACK_WRITE ( add_connection->l2_lookup_entry->tcp_pure_ack, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_DST_MAC_CRC_WRITE ( l2_dst_mac_crc, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_SRC_MAC_CRC_WRITE ( l2_src_mac_crc, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_KEY_EXTEND_WRITE ( 0, nat_cache_lkp_entry_ptr );
        RDD_NAT_CACHE_L2_LKP_ENTRY_VALID_WRITE ( 1/*On*/, nat_cache_lkp_entry_ptr );
     
        /* NAT cache workaround: the context table is wrap around at 64K while the key table is continuous */
        if (nat_cache_entry_index < RDD_NAT_CACHE_EXTENSION_TABLE_SIZE)
        {
            nat_cache_entry_index = RDD_NAT_CACHE_TABLE_SIZE + nat_cache_entry_index;
            nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &(nat_cache_table_ptr->entry[nat_cache_entry_index]);
            create_dup_key = 1;
        }
        else
        {
            nat_cache_entry_index = nat_cache_entry_index & (RDD_NAT_CACHE_TABLE_SIZE - 1);
            nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &(nat_cache_table_ptr->entry[nat_cache_entry_index]);
            create_dup_key = 0;
        }
    } while (create_dup_key);

    /* return the index of the entry in the table */
    add_connection->xo_entry_index = flow_entry_index;

    context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS * )ContextContTableBase;
    context_cont_entry_ptr = &(context_cont_table_ptr->entry[ nat_cache_entry_index ] );
    RDD_CONTEXT_CONTINUATION_ENTRY_FLOW_INDEX_WRITE ( flow_entry_index, context_cont_entry_ptr );

#if defined(CC_RDD_L2_DEBUG)
    {
        __debug_l2("%s, %u: nat_cache_entry_index %u, flow_entry_index %u\n", __FUNCTION__, __LINE__, nat_cache_entry_index, flow_entry_index );
        __debug_l2("%s, %u: DDR conn entry: nat_cache_entry_ptr 0x%p \n", __FUNCTION__, __LINE__, nat_cache_lkp_entry_ptr );
    }
#endif

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return BDMF_ERR_OK;
}


static int f_rdd_l2_nat_cache_lkp_entry_lookup ( uint32_t                  hash_index,
                                                 rdpa_l2_flow_key_t        *lookup_entry,
                                                 uint32_t                  l2_src_mac_crc,
                                                 uint32_t                  l2_dst_mac_crc,
                                                 uint32_t                  *tries_res )
{
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS  *nat_cache_lkp_entry_ptr;
    uint32_t                  tries;
    uint32_t                  nat_cache_entry_index;
    uint32_t                  nat_cache_lkp_entry_valid;
    uint8_t                   nat_cache_lkp_entry_protocol;
    uint16_t                  nat_cache_lkp_entry_tos;
    uint8_t                   nat_cache_lkp_entry_tcp_pure_ack;
    uint32_t                  nat_cache_lkp_entry_src_mac_crc;
    uint32_t                  nat_cache_lkp_entry_dst_mac_crc;
    RDD_NAT_CACHE_TABLE_DTS   *nat_cache_table_ptr = (RDD_NAT_CACHE_TABLE_DTS *)NatCacheTableBase;

    __debug_l2("\n%s, %u: ============================================================ \n", __FUNCTION__, __LINE__); 
    __debug_l2("%s, %u: Input Params: MACSA=", __FUNCTION__, __LINE__);
    print_mac_addr ( &lookup_entry->src_mac); __debug_l2("\n");

    __debug_l2("%s, %u: Input Params: MACDA=", __FUNCTION__, __LINE__);
    print_mac_addr ( &lookup_entry->dst_mac); __debug_l2("\n");

    __debug_l2("%s, %u: Input Params: vtag_num=%u eth_type=0x%04X\n", __FUNCTION__, __LINE__, 
            lookup_entry->vtag_num, lookup_entry->eth_type);

    __debug_l2("%s, %u: vtag0=0x%08X vtag1=0x%08X\n", __FUNCTION__, __LINE__, 
              lookup_entry->vtag0, lookup_entry->vtag1);

    __debug_l2("%s, %u: tos=0x%02X\n", __FUNCTION__, __LINE__, lookup_entry->tos);

    if ( lookup_entry->vtag_num > 2 )
    {
        __debug_l2("%s, %u: invalid params\n", __FUNCTION__, __LINE__); 
        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    for ( tries = 0; tries < RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE; tries++ )
    {
        nat_cache_entry_index = hash_index + tries;

        nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &( nat_cache_table_ptr->entry[ nat_cache_entry_index ] );

        RDD_NAT_CACHE_L2_LKP_ENTRY_VALID_READ ( nat_cache_lkp_entry_valid, nat_cache_lkp_entry_ptr );

        if ( nat_cache_lkp_entry_valid )
        {
            RDD_NAT_CACHE_L2_LKP_ENTRY_PROTOCOL_READ ( nat_cache_lkp_entry_protocol, nat_cache_lkp_entry_ptr );
            RDD_NAT_CACHE_L2_LKP_ENTRY_TOS_READ ( nat_cache_lkp_entry_tos, nat_cache_lkp_entry_ptr );
            RDD_NAT_CACHE_L2_LKP_ENTRY_TCP_PURE_ACK_READ ( nat_cache_lkp_entry_tcp_pure_ack, nat_cache_lkp_entry_ptr );
            RDD_NAT_CACHE_L2_LKP_ENTRY_SRC_MAC_CRC_READ ( nat_cache_lkp_entry_src_mac_crc, nat_cache_lkp_entry_ptr );
            RDD_NAT_CACHE_L2_LKP_ENTRY_DST_MAC_CRC_READ ( nat_cache_lkp_entry_dst_mac_crc, nat_cache_lkp_entry_ptr );

            if ( ( nat_cache_lkp_entry_protocol == lookup_entry->lookup_port ) &&
                 ( nat_cache_lkp_entry_tos == lookup_entry->tos ) &&
                 ( nat_cache_lkp_entry_tcp_pure_ack == lookup_entry->tcp_pure_ack ) &&
                 ( nat_cache_lkp_entry_src_mac_crc == l2_src_mac_crc ) &&
                 ( nat_cache_lkp_entry_dst_mac_crc == l2_dst_mac_crc ) )
            {
                break;
            }
        }
    }

    *tries_res = tries;

    return BDMF_ERR_OK;
}


int rdd_l2_connection_entry_delete ( bdmf_index  flow_entry_index )
{
    RDD_NAT_CACHE_TABLE_DTS      *nat_cache_table_ptr = (RDD_NAT_CACHE_TABLE_DTS *)NatCacheTableBase;
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS  *nat_cache_lkp_entry_ptr, nat_cache_lookup_entry;
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS * )ContextContTableBase;
    RDD_CONTEXT_CONTINUATION_ENTRY_DTS *context_cont_entry_ptr;
    uint32_t                     nat_cache_lkp_entry_valid;
    uint32_t                     context_cont_flow_index;
    uint32_t                     entry_index = RDD_NATC_CONTEXT_TABLE_SIZE;
    uint32_t                     context_entry_connection_table_index;
    unsigned long                flags;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t                      connection_entry_protocol;
    uint8_t                      connection_entry_lookup_port;
    uint16_t                     any_src_port_flow_counter;
    uint32_t                     connection_direction;
    RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS *any_src_port_flow_counter_ptr;

    if ((g_free_flow_entries[flow_entry_index] & RDD_FLOW_ENTRY_VALID) == RDD_FLOW_ENTRY_VALID)
    {
        __debug_l2("%s, %u: flow_entry_index %u deleted\n", __FUNCTION__, __LINE__, (uint32_t) flow_entry_index);
        entry_index = g_free_flow_entries[flow_entry_index] & ~RDD_FLOW_ENTRY_VALID;
    }
    else
    {
        return BDMF_ERR_PARM;
    }

    if ( entry_index >= RDD_NATC_CONTEXT_TABLE_SIZE )
    {
        return BDMF_ERR_PARM;
    }

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    context_cont_entry_ptr = &( context_cont_table_ptr->entry[ entry_index ] );

    RDD_CONTEXT_CONTINUATION_ENTRY_CONNECTION_TABLE_INDEX_READ ( context_entry_connection_table_index, context_cont_entry_ptr );
    RDD_CONTEXT_CONTINUATION_ENTRY_CONNECTION_DIRECTION_READ ( connection_direction, context_cont_entry_ptr );

    /* NAT cache workaround: the context table is wrap around at 64K while the key table is continuous */
    if (entry_index < RDD_NAT_CACHE_EXTENSION_TABLE_SIZE) {
        nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &(nat_cache_table_ptr->entry[RDD_NAT_CACHE_TABLE_SIZE + entry_index]);
        memset(nat_cache_lkp_entry_ptr, 0, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
    }

    nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &( nat_cache_table_ptr->entry[ context_entry_connection_table_index ] );

    RDD_NAT_CACHE_L2_LKP_ENTRY_VALID_READ ( nat_cache_lkp_entry_valid, nat_cache_lkp_entry_ptr );
    RDD_CONTEXT_CONTINUATION_ENTRY_FLOW_INDEX_READ ( context_cont_flow_index, context_cont_entry_ptr );

    if ( nat_cache_lkp_entry_valid && ( flow_entry_index == context_cont_flow_index ) )
    {
        RDD_NAT_CACHE_LKP_ENTRY_PROTOCOL_READ ( connection_entry_protocol, nat_cache_lkp_entry_ptr );

        f_rdd_ipproto_lookup_port_get ( &connection_entry_protocol, &connection_entry_lookup_port );

        if (connection_entry_lookup_port == BL_LILAC_RDD_ANY_BRIDGE_PORT)
        {
            any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 

            MREAD_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
            any_src_port_flow_counter--;
            MWRITE_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );

            any_src_port_flow_counter_ptr = ( RDD_ANY_SRC_PORT_FLOW_COUNTER_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ANY_SRC_PORT_FLOW_COUNTER_ADDRESS ); 

            MREAD_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
            any_src_port_flow_counter--;
            MWRITE_16 ( any_src_port_flow_counter_ptr, any_src_port_flow_counter );
        }

        /* Remove flow context from DDR */
        memcpy(&nat_cache_lookup_entry, nat_cache_lkp_entry_ptr, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));

        memset(nat_cache_lkp_entry_ptr, 0, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));

#if !defined(FIRMWARE_INIT)
        wmb();

        /* Delete the entry in the NAT cache internal memory */
        rdd_nat_cache_submit_command(natc_del, (uint32_t *)&nat_cache_lookup_entry, NULL, NULL);
#endif

        /* Delete continuation flow cache index from cam_lkp tbl */
        f_rdd_cpu_tx_send_message( LILAC_RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY,
                           (connection_direction == rdpa_dir_ds) ?
                           FAST_RUNNER_A : FAST_RUNNER_B,
                           (connection_direction == rdpa_dir_ds) ?
                           RUNNER_PRIVATE_0_OFFSET : RUNNER_PRIVATE_1_OFFSET,
                           context_entry_connection_table_index, 0, 0, BL_LILAC_RDD_WAIT );

        f_rdd_free_context_entry ( entry_index );

        if (rc == BDMF_ERR_OK)
        {
            g_free_flow_entries[flow_entry_index] = 0;
            g_free_flow_entries_number++;
        }
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );

        return rc;
    }
    else
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return BDMF_ERR_INTERNAL;
    }
}


int rdd_l2_connection_entry_search ( rdd_l2_flow_t          *get_connection,
                                     rdpa_traffic_dir       direction,
                                     bdmf_index             *entry_index )
{
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS * )ContextContTableBase;
    RDD_CONTEXT_CONTINUATION_ENTRY_DTS *context_cont_entry_ptr;
    uint32_t                  hash_index, tries = 0;
    uint32_t                  nat_cache_entry_index;
    uint16_t                  flow_entry_index;
    uint32_t                  l2_src_mac_crc;
    uint32_t                  l2_dst_mac_crc;
    unsigned long             flags;
    rdpa_l2_flow_key_t        *l2_lookup_entry = (rdpa_l2_flow_key_t *) get_connection->l2_lookup_entry;

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
        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    l2_src_mac_crc = 0;
    l2_dst_mac_crc = 0;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    rdd_l2_connection_entry_hash( get_connection->l2_lookup_entry, &hash_index, &l2_src_mac_crc, &l2_dst_mac_crc );

    f_rdd_l2_nat_cache_lkp_entry_lookup ( hash_index, get_connection->l2_lookup_entry, l2_src_mac_crc, l2_dst_mac_crc, &tries );

    if ( tries == RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return BDMF_ERR_NOENT;
    }

    nat_cache_entry_index = hash_index + tries;

    context_cont_entry_ptr = &( context_cont_table_ptr->entry[ nat_cache_entry_index ] );
    RDD_CONTEXT_CONTINUATION_ENTRY_FLOW_INDEX_READ ( flow_entry_index, context_cont_entry_ptr );

    *entry_index = flow_entry_index;

    __debug_l2("%s, %u: flow_entry_index=%u, nat_cache_entry_index=%u\n", 
            __FUNCTION__, __LINE__, flow_entry_index, nat_cache_entry_index);
    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );
    return BDMF_ERR_OK;
}


int rdd_l2_connection_entry_get ( rdpa_traffic_dir    direction,
                               uint32_t            entry_index,
                               rdpa_l2_flow_key_t  *nat_cache_lkp_entry,
                               bdmf_index          *flow_entry_index )
{
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS * )ContextContTableBase;
    RDD_CONTEXT_CONTINUATION_ENTRY_DTS *context_cont_entry_ptr;
    RDD_NAT_CACHE_TABLE_DTS      *nat_cache_table_ptr = (RDD_NAT_CACHE_TABLE_DTS *)NatCacheTableBase;
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS  *nat_cache_lkp_entry_ptr;
    uint32_t                     nat_cache_lkp_entry_valid;
    uint16_t                  nat_cache_lkp_entry_tos;
    uint8_t                   nat_cache_lkp_entry_tcp_pure_ack;
    uint32_t                  nat_cache_lkp_entry_src_mac_crc;
    uint32_t                  nat_cache_lkp_entry_dst_mac_crc;
    uint8_t                   nat_cache_lkp_entry_lookup_port;
    uint32_t flow_index;

    nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &( nat_cache_table_ptr->entry[ entry_index ] );

    RDD_NAT_CACHE_L2_LKP_ENTRY_VALID_READ ( nat_cache_lkp_entry_valid, nat_cache_lkp_entry_ptr );

    if ( !nat_cache_lkp_entry_valid )
    {
        return BDMF_ERR_NOENT;
    }

    {
        uint32_t nat_cache_lkp_entry_is_multicast;

        RDD_FC_MCAST_CONNECTION_ENTRY_IS_MULTICAST_READ ( nat_cache_lkp_entry_is_multicast, nat_cache_lkp_entry_ptr );

        if ( nat_cache_lkp_entry_is_multicast )
        {
            return BDMF_ERR_NOENT;
        }
    }

    RDD_NAT_CACHE_L2_LKP_ENTRY_PROTOCOL_READ ( nat_cache_lkp_entry_lookup_port, nat_cache_lkp_entry_ptr );
    RDD_NAT_CACHE_L2_LKP_ENTRY_TOS_READ ( nat_cache_lkp_entry_tos, nat_cache_lkp_entry_ptr );
    RDD_NAT_CACHE_L2_LKP_ENTRY_TCP_PURE_ACK_READ ( nat_cache_lkp_entry_tcp_pure_ack, nat_cache_lkp_entry_ptr );
    RDD_NAT_CACHE_L2_LKP_ENTRY_SRC_MAC_CRC_READ ( nat_cache_lkp_entry_src_mac_crc, nat_cache_lkp_entry_ptr );
    RDD_NAT_CACHE_L2_LKP_ENTRY_DST_MAC_CRC_READ ( nat_cache_lkp_entry_dst_mac_crc, nat_cache_lkp_entry_ptr );

    nat_cache_lkp_entry_src_mac_crc = ntohl(nat_cache_lkp_entry_src_mac_crc );
    nat_cache_lkp_entry_dst_mac_crc = ntohl(nat_cache_lkp_entry_dst_mac_crc );
    memcpy( &nat_cache_lkp_entry->src_mac, &nat_cache_lkp_entry_src_mac_crc, 4 );
    memcpy( &nat_cache_lkp_entry->dst_mac, &nat_cache_lkp_entry_dst_mac_crc, 4 );

    nat_cache_lkp_entry->lookup_port = nat_cache_lkp_entry_lookup_port;
    nat_cache_lkp_entry->tos = nat_cache_lkp_entry_tos;
    nat_cache_lkp_entry->tcp_pure_ack = nat_cache_lkp_entry_tcp_pure_ack;
    nat_cache_lkp_entry->vtag0 = 0;
    nat_cache_lkp_entry->vtag1 = 0;
    nat_cache_lkp_entry->vtag_num = 0;
    nat_cache_lkp_entry->eth_type = 0;
    nat_cache_lkp_entry->dir = 0;

    context_cont_entry_ptr = &( context_cont_table_ptr->entry[ entry_index ] );
    RDD_CONTEXT_CONTINUATION_ENTRY_FLOW_INDEX_READ ( flow_index, context_cont_entry_ptr );
    *flow_entry_index = flow_index;

    return BDMF_ERR_OK;
}

int rdd_l2_context_entry_get ( bdmf_index                  flow_entry_index,
                               rdd_fc_context_t            *context_entry )
{
    RDD_NAT_CACHE_TABLE_DTS      *nat_cache_table_ptr = (RDD_NAT_CACHE_TABLE_DTS *)NatCacheTableBase;
    RDD_NATC_CONTEXT_TABLE_DTS   *context_table_ptr = ( RDD_NATC_CONTEXT_TABLE_DTS * )ContextTableBase;
    RDD_NATC_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS * )ContextContTableBase;
    RDD_CONTEXT_CONTINUATION_ENTRY_DTS *context_cont_entry_ptr;
    uint32_t                     valid;
    uint32_t                     multicast_flag;
    uint32_t                     connection_direction;
    int                          continuation_flag;
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS  *nat_cache_lkp_entry_ptr, nat_cache_lookup_entry;
    uint32_t                     connection_table_index;
    uint32_t                     entry_index = RDD_NATC_CONTEXT_TABLE_SIZE;

    if ((g_free_flow_entries[flow_entry_index] & RDD_FLOW_ENTRY_VALID) == RDD_FLOW_ENTRY_VALID)
    {
        entry_index = g_free_flow_entries[flow_entry_index] & ~RDD_FLOW_ENTRY_VALID;
    }
    else
    {
        return BDMF_ERR_PARM;
    }

    if ( entry_index >= RDD_NATC_CONTEXT_TABLE_SIZE )
    {
        return ( BDMF_ERR_PARM );
    }

    context_entry_ptr = &(context_table_ptr->entry[ entry_index ] );
    context_cont_entry_ptr = &( context_cont_table_ptr->entry[ entry_index ] );

    RDD_CONTEXT_CONTINUATION_ENTRY_VALID_READ (valid, context_cont_entry_ptr );

    if(!valid)
    {
        return BDMF_ERR_NOENT;
    }

    RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_READ( multicast_flag, context_entry_ptr );
    RDD_CONTEXT_CONTINUATION_ENTRY_CONNECTION_DIRECTION_READ ( connection_direction, context_cont_entry_ptr );
    RDD_CONTEXT_CONTINUATION_ENTRY_CONNECTION_TABLE_INDEX_READ ( connection_table_index, context_cont_entry_ptr );

    nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &( nat_cache_table_ptr->entry[ connection_table_index ] );
    RDD_NAT_CACHE_L2_LKP_ENTRY_VALID_READ ( valid, nat_cache_lkp_entry_ptr );

    if( !multicast_flag )
    {
        uint32_t context_index;
        uint32_t i;

        if ( valid )
        {
            RDD_CONTEXT_CONTINUATION_ENTRY_FLOW_INDEX_READ ( context_index, context_cont_entry_ptr );

            if ( flow_entry_index == context_index )
            {
                /* look for the entry in the NAT cache internal memory, if found then statistics is also there */
                int rc = BDMF_ERR_OK;
                uint32_t hit_count, byte_count;
#if !defined(FIRMWARE_INIT)
                unsigned long flags;
#endif

                memcpy(&nat_cache_lookup_entry, nat_cache_lkp_entry_ptr, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#if !defined(FIRMWARE_INIT)
                bdmf_fastlock_lock_irq ( &int_lock_irq, flags );
                rc = rdd_nat_cache_submit_command(natc_lookup, (uint32_t *)&nat_cache_lookup_entry, &hit_count, &byte_count);
                bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
#else
                hit_count = 0;
                byte_count = 0;
#endif
                if (rc == BDMF_ERR_OK)
                {
                    context_entry->fc_ucast_flow_context_entry.flow_hits = hit_count;
                    context_entry->fc_ucast_flow_context_entry.flow_bytes = byte_count;
                }
                else
                {
                    RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_READ( context_entry->fc_ucast_flow_context_entry.flow_hits, context_entry_ptr );
                    RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_READ( context_entry->fc_ucast_flow_context_entry.flow_bytes, context_entry_ptr );
                }

                context_entry->fc_ucast_flow_context_entry.multicast_flag = multicast_flag;
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_ROUTED_READ( context_entry->fc_ucast_flow_context_entry.is_routed, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_L2_ACCEL_READ( context_entry->fc_ucast_flow_context_entry.is_l2_accel, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_EGRESS_PHY_READ( context_entry->fc_ucast_flow_context_entry.egress_phy, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_HIGH_PRIO_READ( context_entry->fc_ucast_flow_context_entry.is_high_prio, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_MTU_READ( context_entry->fc_ucast_flow_context_entry.mtu, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_TOS_MANGLE_READ( context_entry->fc_ucast_flow_context_entry.is_tos_mangle, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_TOS_READ( context_entry->fc_ucast_flow_context_entry.tos, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_SERVICE_QUEUE_ID_READ( context_entry->fc_ucast_flow_context_entry.service_queue_id, context_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_UNICAST_WFD_ANY_READ(context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any, context_entry_ptr);
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_DROP_READ(context_entry->fc_ucast_flow_context_entry.drop, context_entry_ptr);
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_PRIORITY_READ(context_entry->fc_ucast_flow_context_entry.priority, context_entry_ptr);
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_WFD_PRIO_READ(context_entry->fc_ucast_flow_context_entry.wfd_prio, context_entry_ptr);
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_WFD_IDX_READ(context_entry->fc_ucast_flow_context_entry.wfd_idx, context_entry_ptr);
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_PATHSTAT_IDX_READ(context_entry->fc_ucast_flow_context_entry.pathstat_idx, context_entry_ptr);
                RDD_CONTEXT_CONTINUATION_ENTRY_IS_UNICAST_WFD_NIC_READ(context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic, context_cont_entry_ptr);

                if (context_entry->fc_ucast_flow_context_entry.egress_phy == rdd_egress_phy_wlan)
                {
                    if (context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_any)
                    {
                        if (context_entry->fc_ucast_flow_context_entry.is_unicast_wfd_nic)
                        {
                            RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_CHAIN_IDX_READ(context_entry->fc_ucast_flow_context_wfd_nic_entry.chain_idx, context_entry_ptr);
                        }
                        else
                        {
                            RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_WIFI_SSID_READ(context_entry->fc_ucast_flow_context_wfd_dhd_entry.wifi_ssid, context_entry_ptr);
                            RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_FLOW_RING_ID_READ(context_entry->fc_ucast_flow_context_wfd_dhd_entry.flow_ring_id, context_entry_ptr);
                        }
                    }
#if defined(CONFIG_DHD_RUNNER)
                    else
                    {
                        RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_RADIO_IDX_READ(context_entry->fc_ucast_flow_context_rnr_dhd_entry.radio_idx, context_entry_ptr);
                        RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_WIFI_SSID_READ(context_entry->fc_ucast_flow_context_rnr_dhd_entry.wifi_ssid, context_entry_ptr);
                        RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_FLOW_RING_ID_READ(context_entry->fc_ucast_flow_context_rnr_dhd_entry.flow_ring_id, context_entry_ptr);
                    }
#endif
                }
                else
                {
                    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_EGRESS_PORT_READ(context_entry->fc_ucast_flow_context_eth_xtm_entry.egress_port, context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_TRAFFIC_CLASS_READ(context_entry->fc_ucast_flow_context_eth_xtm_entry.traffic_class, context_entry_ptr);
                    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_RATE_CONTROLLER_READ(context_entry->fc_ucast_flow_context_eth_xtm_entry.rate_controller, context_entry_ptr);
                }

                context_entry->fc_ucast_flow_context_entry.connection_direction = connection_direction;
                context_entry->fc_ucast_flow_context_entry.connection_table_index = connection_table_index;

                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_CONTEXT_CONTINUATION_FLAG_READ ( continuation_flag, context_entry_ptr );
                if (continuation_flag)
                {
                    context_entry->fc_ucast_flow_context_entry.command_list_length_64 = RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER/8;

                    for ( i = 0; i < RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER; i++ )
                    {
                        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_READ ( context_entry->fc_ucast_flow_context_entry.command_list[ i ], context_entry_ptr, i );
                    }

                    for ( i = 0; i < RDD_CONTEXT_CONTINUATION_ENTRY_COMMAND_LIST_NUMBER; i++ )
                    {
                        uint32_t cmd_list_idx = i + RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER; 
                        RDD_CONTEXT_CONTINUATION_ENTRY_COMMAND_LIST_READ ( context_entry->fc_ucast_flow_context_entry.command_list[cmd_list_idx], context_cont_entry_ptr, i );
                    }
                }
                else
                {
                    context_entry->fc_ucast_flow_context_entry.command_list_length_64 = (RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER+4)/8;
                    for ( i = 0; i < RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER+4; i++ )
                    {
                        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_READ ( context_entry->fc_ucast_flow_context_entry.command_list[ i ], context_entry_ptr, i );
                    }
                }

                RDD_CONTEXT_CONTINUATION_ENTRY_VALID_READ (context_entry->fc_ucast_flow_context_entry.valid, context_cont_entry_ptr );
                RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IP_ADDRESSES_TABLE_INDEX_READ( context_entry->fc_ucast_flow_context_entry.ip_addresses_table_index, context_entry_ptr );
            }
            else
            {
                return BDMF_ERR_NOENT;
            }
        }
        else
        {
            return BDMF_ERR_NOENT;
        }
    }

    return BDMF_ERR_OK;
}

int rdd_l2_context_entry_flwstat_get ( bdmf_index                  flow_entry_index,
                                       rdd_fc_context_t            *context_entry )
{
    RDD_NAT_CACHE_TABLE_DTS      *nat_cache_table_ptr = (RDD_NAT_CACHE_TABLE_DTS *)NatCacheTableBase;
    RDD_NATC_CONTEXT_TABLE_DTS   *context_table_ptr = ( RDD_NATC_CONTEXT_TABLE_DTS * )ContextTableBase;
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS * )ContextContTableBase;
    RDD_NATC_CONTEXT_ENTRY_UNION_DTS  *context_entry_ptr;
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS  *nat_cache_lkp_entry_ptr, nat_cache_lookup_entry;
    uint32_t                     entry_index = RDD_NATC_CONTEXT_TABLE_SIZE;
    RDD_CONTEXT_CONTINUATION_ENTRY_DTS *context_cont_entry_ptr;
    uint32_t                     connection_table_index;
    /* look for the entry in the NAT cache internal memory, if found then statistics is also there */
    int rc = BDMF_ERR_OK;
    uint32_t hit_count, byte_count;
    uint32_t valid;
    uint32_t multicast_flag;
#if !defined(FIRMWARE_INIT)
    unsigned long flags;
#endif

    if ((g_free_flow_entries[flow_entry_index] & RDD_FLOW_ENTRY_VALID) == RDD_FLOW_ENTRY_VALID)
    {
        entry_index = g_free_flow_entries[flow_entry_index] & ~RDD_FLOW_ENTRY_VALID;
    }
    else
    {
        return BDMF_ERR_PARM;
    }

    if ( entry_index >= RDD_NATC_CONTEXT_TABLE_SIZE )
    {
        return ( BDMF_ERR_PARM );
    }

    context_entry_ptr = &(context_table_ptr->entry[ entry_index ] );
    context_cont_entry_ptr = &( context_cont_table_ptr->entry[ entry_index ] );

    RDD_CONTEXT_CONTINUATION_ENTRY_VALID_READ (valid, context_cont_entry_ptr );

    if(!valid)
    {
        return BDMF_ERR_NOENT;
    }
    RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_MULTICAST_FLAG_READ(multicast_flag, context_entry_ptr );
    RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_IS_L2_ACCEL_READ( context_entry->fc_ucast_flow_context_entry.is_l2_accel, context_entry_ptr );

    context_entry->fc_ucast_flow_context_entry.multicast_flag = multicast_flag;
    context_entry->fc_mcast_flow_context_entry.multicast_flag = multicast_flag;
    RDD_CONTEXT_CONTINUATION_ENTRY_CONNECTION_TABLE_INDEX_READ ( connection_table_index, context_cont_entry_ptr );

    nat_cache_lkp_entry_ptr = (RDD_NAT_CACHE_L2_LKP_ENTRY_DTS *) &( nat_cache_table_ptr->entry[ connection_table_index ] );

    memcpy(&nat_cache_lookup_entry, nat_cache_lkp_entry_ptr, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#if !defined(FIRMWARE_INIT)
    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );
    rc = rdd_nat_cache_submit_command(natc_lookup, (uint32_t *)&nat_cache_lookup_entry, &hit_count, &byte_count);
    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
#else
    hit_count = 0;
    byte_count = 0;
#endif
    if (rc == BDMF_ERR_OK)
    {
        context_entry->fc_ucast_flow_context_entry.flow_hits = hit_count;
        context_entry->fc_ucast_flow_context_entry.flow_bytes = byte_count;
    }
    else
    {
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_HITS_READ( context_entry->fc_ucast_flow_context_entry.flow_hits, context_entry_ptr );
        RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_FLOW_BYTES_READ( context_entry->fc_ucast_flow_context_entry.flow_bytes, context_entry_ptr );
    }
    return BDMF_ERR_OK;
}

int rdd_l2_context_entry_modify ( rdd_fc_context_t *context_entry,
                                  bdmf_index       flow_entry_index )
{
    uint32_t entry_index;
    unsigned long flags;

    if ((g_free_flow_entries[flow_entry_index] & RDD_FLOW_ENTRY_VALID) == RDD_FLOW_ENTRY_VALID)
    {
        entry_index = g_free_flow_entries[flow_entry_index] & ~RDD_FLOW_ENTRY_VALID;
    }
    else
    {
        return BDMF_ERR_PARM;
    }

    if ( entry_index >= RDD_NATC_CONTEXT_TABLE_SIZE )
    {
        return BDMF_ERR_PARM;
    }

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );
    context_entry->fc_mcast_flow_context_entry.valid = 1;
    f_rdd_l2_context_entry_write ( context_entry, entry_index, 0 );
    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );

    return BDMF_ERR_OK;
}


