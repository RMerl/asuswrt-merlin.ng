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
#ifdef CONFIG_DHD_RUNNER
#include "dhd_defs.h"
#endif

#define _RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE  RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE2

/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

uint16_t  g_ingress_packet_descriptors_counter;

#if defined(FIRMWARE_INIT)
extern uint32_t  GenericClassifierTable;
#endif

#if !defined(FIRMWARE_INIT)
static uint8_t                                 g_dummy_read;
#endif
extern uint8_t                                 g_broadcom_switch_mode;
extern BL_LILAC_RDD_BRIDGE_PORT_DTE            g_broadcom_switch_physical_port;
extern BL_LILAC_RDD_WAN_PHYSICAL_PORT_DTE      g_wan_physical_port;

extern RDD_WAN_TX_POINTERS_TABLE_DTS           *wan_tx_pointers_table_ptr;
extern rdpa_bpm_buffer_size_t g_bpm_buffer_size;

#ifdef FIRMWARE_INIT
extern uint8_t *cpu_rx_ring_base;
#endif

#if defined(CONFIG_DHD_RUNNER)
extern bdmf_boolean is_dhd_enabled[];
#endif

#if !defined(RDD_BASIC)
#if !defined(FIRMWARE_INIT)
static inline int32_t f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port )
{
    switch ( xi_bridge_port )
    {
    case BL_LILAC_RDD_WAN0_BRIDGE_PORT: // DSL
    case BL_LILAC_RDD_WAN1_BRIDGE_PORT: // DSL

        return ( DRV_BPM_SP_GPON );

    case BL_LILAC_RDD_LAN0_BRIDGE_PORT:

        return ( DRV_BPM_SP_EMAC0 );

    case BL_LILAC_RDD_LAN1_BRIDGE_PORT:

        return ( DRV_BPM_SP_EMAC1 );

    case BL_LILAC_RDD_LAN2_BRIDGE_PORT:

        return ( DRV_BPM_SP_EMAC2 );

    case BL_LILAC_RDD_LAN3_BRIDGE_PORT:

        return ( DRV_BPM_SP_EMAC3 );

    case BL_LILAC_RDD_LAN4_BRIDGE_PORT:

        return ( DRV_BPM_SP_EMAC4 );

    case BL_LILAC_RDD_PCI_BRIDGE_PORT:

        return ( DRV_BPM_SP_PCI0 );

    default:

        return ( 0 );
    }

    return ( 0 );
}
#endif /* !defined(FIRMWARE_INIT) */
#endif /* !defined(RDD_BASIC) */

static inline rdpa_cpu_reason cpu_reason_to_cpu_per_port_reason_index(rdpa_cpu_reason xi_cpu_reason)
{
    switch ( xi_cpu_reason )
    {
    case rdpa_cpu_rx_reason_mcast:
        return 0;
    case rdpa_cpu_rx_reason_bcast:
        return 1;
    case rdpa_cpu_rx_reason_unknown_da:
        return 2;
    default:
        break;
    }
    return 3;
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_initialize ( void )
{
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS               *cpu_reason_to_cpu_rx_queue_table_ptr;
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS               *cpu_reason_to_cpu_rx_queue_entry_ptr;
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS                      *cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS                      *cpu_reason_to_meter_entry_ptr;
#if defined(FIRMWARE_INIT)
    RDD_CPU_RX_DESCRIPTOR_DTS                              *cpu_rx_descriptor_ptr;
    uint32_t                                               host_buffer_address;
#endif
    uint16_t                                               *cpu_rx_ingress_queue_ptr;
    uint8_t                                                cpu_reason;
    uint8_t                                                cpu_queue;
#if defined(FIRMWARE_INIT)
    uint32_t                                               i;

    /* Init Rings */
    cpu_rx_descriptor_ptr = ( RDD_CPU_RX_DESCRIPTOR_DTS * )cpu_rx_ring_base;

    host_buffer_address = SIMULATOR_DDR_RING_OFFSET + RDD_RING_DESCRIPTORS_TABLE_SIZE * SIMULATOR_DDR_RING_NUM_OF_ENTRIES * sizeof ( RDD_CPU_RX_DESCRIPTOR_DTS );

    for ( i = 0; i < RDD_RING_DESCRIPTORS_TABLE_SIZE * 10; i++, cpu_rx_descriptor_ptr++, host_buffer_address += RDD_SIMULATION_PACKET_BUFFER_SIZE )
    {
        RDD_CPU_RX_DESCRIPTOR_HOST_DATA_BUFFER_POINTER_WRITE ( host_buffer_address, cpu_rx_descriptor_ptr );
    }
#endif

    cpu_rx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_FAST_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( cpu_rx_ingress_queue_ptr, DS_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS );

    cpu_rx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_FAST_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( cpu_rx_ingress_queue_ptr, US_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS );


    cpu_rx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_PICO_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( cpu_rx_ingress_queue_ptr, DS_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS );

    cpu_rx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_PICO_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( cpu_rx_ingress_queue_ptr, US_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS );

    cpu_reason_to_cpu_rx_queue_table_ptr = ( RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS );

    /* set cpu reason_direct_flow to queue 0 for backward compatibility */
    cpu_reason = rdpa_cpu_rx_reason_direct_flow;
    cpu_queue = BL_LILAC_RDD_CPU_RX_QUEUE_0;
    cpu_reason_to_cpu_rx_queue_entry_ptr = &( cpu_reason_to_cpu_rx_queue_table_ptr->entry[ 0 ] [ cpu_reason ] );
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE ( cpu_queue, cpu_reason_to_cpu_rx_queue_entry_ptr );
    cpu_reason_to_cpu_rx_queue_entry_ptr = &( cpu_reason_to_cpu_rx_queue_table_ptr->entry[ 1 ] [ cpu_reason ] );
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE ( cpu_queue, cpu_reason_to_cpu_rx_queue_entry_ptr );

    for ( cpu_reason = rdpa_cpu_rx_reason_oam; cpu_reason < rdpa_cpu_reason__num_of; cpu_reason++ )
    {
        cpu_reason_to_meter_table_ptr = ( RDD_DS_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_REASON_TO_METER_TABLE_ADDRESS );

        cpu_reason_to_meter_entry_ptr = &( cpu_reason_to_meter_table_ptr->entry[ cpu_reason ] );

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( BL_LILAC_RDD_CPU_METER_DISABLE, cpu_reason_to_meter_entry_ptr );

        cpu_reason_to_meter_table_ptr = ( RDD_DS_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_REASON_TO_METER_TABLE_ADDRESS );

        cpu_reason_to_meter_entry_ptr = &( cpu_reason_to_meter_table_ptr->entry[ cpu_reason ] );

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( BL_LILAC_RDD_CPU_METER_DISABLE, cpu_reason_to_meter_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}

#if !defined(FIRMWARE_INIT)
extern cpu_tx_skb_free_indexes_cache_t g_cpu_tx_skb_free_indexes_cache;

static inline BL_LILAC_RDD_ERROR_DTE f_rdd_cpu_tx_skb_free_cache_initialize ( void )
{
    g_cpu_tx_skb_free_indexes_cache.write = 0;
    g_cpu_tx_skb_free_indexes_cache.read = 0;
    g_cpu_tx_skb_free_indexes_cache.count = 0;
    g_cpu_tx_skb_free_indexes_cache.data = (uint16_t *)CACHED_MALLOC_ATOMIC(sizeof(RDD_FREE_SKB_INDEXES_FIFO_ENTRY_DTS) * g_cpu_tx_abs_packet_limit);

    if (g_cpu_tx_skb_free_indexes_cache.data == NULL)
        return ( BL_LILAC_RDD_ERROR_MALLOC_FAILED );
    return ( BL_LILAC_RDD_OK );
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_initialize ( void )
{
    uint32_t *ih_header_descriptor_ptr;
    uint32_t ih_header_descriptor[2];
    uint32_t *ih_buffer_bbh_ptr;
#if defined(CONFIG_RUNNER_GSO)
    uint16_t *gso_queue_ptr_ptr;
#endif
#if defined(CONFIG_RUNNER_IPSEC)
    uint16_t *ipsec_queue_ptr_ptr;
#endif
    int i;

    ih_header_descriptor_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS );

    ih_header_descriptor[ 0 ] = ( LILAC_RDD_ON << 5 ) + ( CPU_TX_FAST_THREAD_NUMBER << 6 );

    ih_header_descriptor[ 1 ] = WAN_SRC_PORT + ( LILAC_RDD_IH_HEADER_LENGTH << 5 ) + ( LILAC_RDD_RUNNER_A_IH_BUFFER << 20 );

    MWRITE_32( ( ( uint8_t * )ih_header_descriptor_ptr + 0 ), ih_header_descriptor[ 0 ] );
    MWRITE_32( ( ( uint8_t * )ih_header_descriptor_ptr + 4 ), ih_header_descriptor[ 1 ] );

    ih_header_descriptor_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS );
    for (i = 0; i < 3; i++)
    {
        ih_header_descriptor[ 0 ] = LILAC_RDD_ON << 5;

        ih_header_descriptor[ 1 ] = ( LILAC_RDD_IH_HEADER_LENGTH << 5 ) + ( LILAC_RDD_RUNNER_B_IH_BUFFER << 20 );

        MWRITE_32( ( ( uint8_t * )ih_header_descriptor_ptr + 0 ), ih_header_descriptor[ 0 ] );
        MWRITE_32( ( ( uint8_t * )ih_header_descriptor_ptr + 4 ), ih_header_descriptor[ 1 ] );
        ih_header_descriptor_ptr += 2;
    }

    ih_buffer_bbh_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + IH_BUFFER_BBH_POINTER_ADDRESS );

    MWRITE_32( ih_buffer_bbh_ptr, ( ( BBH_PERIPHERAL_IH << 16 ) | ( LILAC_RDD_IH_BUFFER_BBH_ADDRESS + LILAC_RDD_RUNNER_B_IH_BUFFER_BBH_OFFSET ) ) );

    g_cpu_tx_queue_write_ptr[ FAST_RUNNER_A ] = CPU_TX_FAST_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[ FAST_RUNNER_B ] = CPU_TX_FAST_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[ PICO_RUNNER_A ] = CPU_TX_PICO_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[ PICO_RUNNER_B ] = CPU_TX_PICO_QUEUE_ADDRESS;

    g_cpu_tx_queue_abs_data_ptr_write_ptr[ FAST_RUNNER_A ] = DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[ FAST_RUNNER_B ] = US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[ PICO_RUNNER_A ] = DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[ PICO_RUNNER_B ] = US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;

#if defined(CONFIG_RUNNER_GSO)
    gso_queue_ptr_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + GSO_PICO_QUEUE_PTR_ADDRESS );
    MWRITE_16( gso_queue_ptr_ptr, GSO_PICO_QUEUE_ADDRESS );
#endif

#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    f_rdd_cpu_tx_skb_free_cache_initialize();
    f_rdd_initialize_skb_free_indexes_cache();
    rdd_cpu_tx_free_skb_timer_config();
#endif

#if defined(CONFIG_RUNNER_IPSEC)
    ipsec_queue_ptr_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPSEC_DS_QUEUE_PTR_ADDRESS );
    MWRITE_16( ipsec_queue_ptr_ptr, IPSEC_DS_QUEUE_ADDRESS );
#endif

#if defined(CONFIG_BCM_PKTRUNNER_GSO) && defined(CONFIG_RUNNER_GSO)
    bdmf_gso_desc_pool_create(RUNNER_MAX_GSO_DESC );
#endif
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_TYPE  xi_msg_type,
                                                   LILAC_RDD_RUNNER_INDEX_DTS     xi_runner_index,
                                                   uint32_t                       xi_sram_base,
                                                   uint32_t                       xi_parameter_1,
                                                   uint32_t                       xi_parameter_2,
                                                   uint32_t                       xi_parameter_3,
                                                   BL_LILAC_RDD_CPU_WAIT_DTE      xi_wait )
{
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    RDD_CPU_TX_DESCRIPTOR_DTS   *cpu_tx_descriptor_ptr;
#if !defined(BDMF_SYSTEM_SIM)
    uint32_t                    cpu_tx_descriptor_valid;
#endif

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( xi_sram_base ) + g_cpu_tx_queue_write_ptr[ xi_runner_index ] );

    if( g_cpu_tx_queue_free_counter[ xi_runner_index ] == 0 )
    {
        f_rdd_get_tx_descriptor_free_count ( xi_runner_index, cpu_tx_descriptor_ptr );

        if( g_cpu_tx_queue_free_counter[ xi_runner_index ] == 0 )
            return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

    RDD_CPU_TX_MESSAGE_DESCRIPTOR_COMMAND_WRITE ( LILAC_RDD_CPU_TX_COMMAND_MESSAGE, cpu_tx_descriptor_ptr );
    RDD_CPU_TX_MESSAGE_DESCRIPTOR_MESSAGE_TYPE_WRITE ( xi_msg_type, cpu_tx_descriptor_ptr );

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_FLUSH_GPON_QUEUE )
    {
        RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_FLUSH_ETH_QUEUE )
    {
        RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DESCRIPTOR_QUEUE_WRITE ( xi_parameter_2, cpu_tx_descriptor_ptr );
    }

#ifdef CONFIG_DHD_RUNNER
    if ( is_dhd_enabled[(xi_parameter_2 >> 14) & 0x3] && xi_runner_index == PICO_RUNNER_A && xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_DHD_MESSAGE )
    {
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DHD_MSG_TYPE_WRITE( xi_parameter_1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_FLOW_RING_ID_WRITE( (xi_parameter_2 & 0x3FF), cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_RADIO_IDX_WRITE( (xi_parameter_2 >> 14) & 0x3, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_VALID_WRITE((xi_parameter_2 >> 31) & 0x1, cpu_tx_descriptor_ptr);
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_WRITE((xi_parameter_2 >> 16) & 0x3FF, cpu_tx_descriptor_ptr);
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DISABLED_WRITE ( xi_parameter_3, cpu_tx_descriptor_ptr );
    }
#endif

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_RELEASE_SKB_BUFFERS )
    {
        RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_FLOW_PM_COUNTERS_GET ) || ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_RX_FLOW_PM_COUNTERS_GET ) || ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_TX_FLOW_PM_COUNTERS_GET ) )
    {
        RDD_CPU_TX_DESCRIPTOR_FLOW_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET )
    {
        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_PM_COUNTER_GET )
    {
        RDD_CPU_TX_DESCRIPTOR_GROUP_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DESCRIPTOR_COUNTER_WRITE ( xi_parameter_2, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_WRITE ( xi_parameter_3, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_IPTV_MAC_COUNTER_GET )
    {
        RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if (( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_UPDATE_PD_POOL_QUOTA )
                                     ||
        ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_UPDATE_US_PD_POOL_QUOTA ))

    {
        RDD_CPU_TX_UPDATE_PD_POOL_QUOTA_MESSAGE_DESCRIPTOR_GUARANTEED_FREE_COUNT_INCR_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_UPDATE_PD_POOL_QUOTA_MESSAGE_DESCRIPTOR_GUARANTEED_FREE_COUNT_DELTA_WRITE ( xi_parameter_2, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_MIRRORING_MODE_CONFIG
#ifdef CONFIG_DHD_RUNNER
        && xi_runner_index == FAST_RUNNER_B
#endif
        )
    {
        RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY )
    {
        RDD_CPU_TX_DESCRIPTOR_CONTEXT_INDEX_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_RING_DESTROY )
    {
        RDD_CPU_TX_DESCRIPTOR_MESSAGE_PARAMETER_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    RDD_CPU_TX_MESSAGE_DESCRIPTOR_VALID_WRITE ( LILAC_RDD_TRUE, cpu_tx_descriptor_ptr );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ xi_runner_index ] += sizeof(RDD_CPU_TX_DESCRIPTOR_DTS);
    g_cpu_tx_queue_write_ptr[ xi_runner_index ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;
    g_cpu_tx_queue_free_counter[ xi_runner_index ]--;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    if ( xi_runner_index == FAST_RUNNER_A || xi_runner_index == FAST_RUNNER_B )
    {
        runner_cpu_wakeup_register.req_trgt = CPU_TX_FAST_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = CPU_TX_FAST_THREAD_NUMBER % 32;
        runner_cpu_wakeup_register.urgent_req = LILAC_RDD_FALSE;
    }
    else
    {
        runner_cpu_wakeup_register.req_trgt = CPU_TX_PICO_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = CPU_TX_PICO_THREAD_NUMBER % 32;
        runner_cpu_wakeup_register.urgent_req = LILAC_RDD_FALSE;
    }

    if ( xi_runner_index == FAST_RUNNER_A || xi_runner_index == PICO_RUNNER_A )
    {
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
    }
    else
    {
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
    }

#if !defined(BDMF_SYSTEM_SIM)
    if ( xi_wait == BL_LILAC_RDD_WAIT )
    {
        /* wait for the cpu tx thread to finish the current message */
        do
        {
            RDD_CPU_TX_MESSAGE_DESCRIPTOR_VALID_READ ( cpu_tx_descriptor_valid, ( ( volatile RDD_CPU_TX_DESCRIPTOR_DTS * )cpu_tx_descriptor_ptr ) );
        }
        while ( cpu_tx_descriptor_valid == LILAC_RDD_TRUE );
    }
#endif

    return ( BL_LILAC_RDD_OK );
}
BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_queue_discard_get ( BL_LILAC_RDD_CPU_RX_QUEUE_DTE  xi_ring_id,
                                                      uint16_t                       *xo_number_of_packets )
{
    RDD_RING_DESCRIPTORS_TABLE_DTS  *ring_table_ptr;
    RDD_RING_DESCRIPTOR_DTS         *ring_descriptor_ptr;
    unsigned long                   flags;

    /* check the validity of the input parameters - CPU-RX queue index */
    if ( xi_ring_id >= RDD_RING_DESCRIPTORS_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_ILLEGAL );
    }

    ring_table_ptr = ( RDD_RING_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RING_DESCRIPTORS_TABLE_ADDRESS );

    ring_descriptor_ptr = &(ring_table_ptr->entry[ xi_ring_id ]);

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    RDD_RING_DESCRIPTOR_DROP_COUNTER_READ ( *xo_number_of_packets, ring_descriptor_ptr );
    RDD_RING_DESCRIPTOR_DROP_COUNTER_WRITE ( 0, ring_descriptor_ptr );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE _rdd_cpu_reason_to_cpu_rx_queue ( rdpa_cpu_reason  xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_RX_QUEUE_DTE   xi_queue_id,
                                                        rdpa_traffic_dir                xi_direction,
                                                        uint32_t                        xi_table_index )
{
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS  *cpu_reason_to_cpu_rx_queue_table_ptr;
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS  *cpu_reason_to_cpu_rx_queue_entry_ptr;
    uint8_t                                   cpu_queue;

    /* check the validity of the input parameters - CPU-RX reason */
    if ( xi_cpu_reason >= _RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CPU_RX_REASON_ILLEGAL );
    }

    if ( (xi_direction == rdpa_dir_ds && xi_table_index > CPU_REASON_WAN1_TABLE_INDEX) ||
         (xi_direction == rdpa_dir_us && xi_table_index > CPU_REASON_LAN_TABLE_INDEX) )
    {
        return ( BL_LILAC_RDD_ERROR_CPU_RX_REASON_ILLEGAL );
    }

    /* check the validity of the input parameters - CPU-RX queue-id */
    if ( xi_queue_id >= CPU_RX_NUMBER_OF_QUEUES )
    {
        return ( BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_ILLEGAL );
    }

    if ( xi_direction == rdpa_dir_ds )
    {
        cpu_reason_to_cpu_rx_queue_table_ptr = ( RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS );
    }
    else
    {
        cpu_reason_to_cpu_rx_queue_table_ptr = ( RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );
    }

    cpu_reason_to_cpu_rx_queue_entry_ptr = &( cpu_reason_to_cpu_rx_queue_table_ptr->entry[xi_table_index] [ xi_cpu_reason ] );

    cpu_queue = xi_queue_id;

    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE ( cpu_queue, cpu_reason_to_cpu_rx_queue_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}

#if defined(WL4908)
BL_LILAC_RDD_ERROR_DTE rdd_cpu_reason_to_cpu_rx_queue ( rdpa_cpu_reason  xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_RX_QUEUE_DTE   xi_queue_id,
                                                        rdpa_traffic_dir                xi_direction)
{
    return _rdd_cpu_reason_to_cpu_rx_queue(xi_cpu_reason, xi_queue_id, xi_direction, 0);
}
#else /* defined(WL4908) */
BL_LILAC_RDD_ERROR_DTE rdd_cpu_reason_to_cpu_rx_queue ( rdpa_cpu_reason  xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_RX_QUEUE_DTE   xi_queue_id,
                                                        rdpa_traffic_dir                xi_direction,
                                                        uint32_t                        xi_table_index )
{
    return _rdd_cpu_reason_to_cpu_rx_queue(xi_cpu_reason, xi_queue_id, xi_direction, xi_table_index);
}
#endif

static BL_LILAC_RDD_ERROR_DTE f_bl_lilac_rdd_cpu_reason_and_src_bridge_port_to_cpu_rx_meter_cfg ( rdpa_cpu_reason  xi_cpu_reason,
                                                                                                  BL_LILAC_RDD_CPU_METER_DTE      xi_cpu_meter,
                                                                                                  BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_src_port )
{
    RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS  *cpu_reason_and_src_bridge_port_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS                      *cpu_reason_and_src_bridge_port_to_meter_entry_ptr;
    uint32_t                                               cpu_reason_per_port_index;

    cpu_reason_and_src_bridge_port_to_meter_table_ptr = ( RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_ADDRESS );

    cpu_reason_per_port_index = 0;

    cpu_reason_per_port_index = cpu_reason_to_cpu_per_port_reason_index(xi_cpu_reason);

    cpu_reason_and_src_bridge_port_to_meter_entry_ptr = &( cpu_reason_and_src_bridge_port_to_meter_table_ptr->entry[ cpu_reason_per_port_index ][ xi_src_port ] );

    RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( xi_cpu_meter, cpu_reason_and_src_bridge_port_to_meter_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_bl_lilac_rdd_cpu_reason_and_src_bridge_port_to_cpu_rx_meter_clear ( rdpa_cpu_reason  xi_cpu_reason,
                                                                                                    BL_LILAC_RDD_CPU_METER_DTE      xi_cpu_meter,
                                                                                                    BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_src_port )
{
    RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS  *cpu_reason_and_src_bridge_port_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS                      *cpu_reason_and_src_bridge_port_to_meter_entry_ptr;
    BL_LILAC_RDD_CPU_METER_DTE                             curr_cpu_meter;
    uint32_t                                               cpu_reason_per_port_index;

    cpu_reason_and_src_bridge_port_to_meter_table_ptr = ( RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) +
                                                                                                                    CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_ADDRESS );

    cpu_reason_per_port_index = 0;

    cpu_reason_per_port_index = cpu_reason_to_cpu_per_port_reason_index(xi_cpu_reason);

    cpu_reason_and_src_bridge_port_to_meter_entry_ptr = &( cpu_reason_and_src_bridge_port_to_meter_table_ptr->entry[ cpu_reason_per_port_index ][ xi_src_port ] );

    RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_READ ( curr_cpu_meter, cpu_reason_and_src_bridge_port_to_meter_entry_ptr );

    if ( curr_cpu_meter == xi_cpu_meter )
    {
        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( BL_LILAC_RDD_CPU_METER_DISABLE, cpu_reason_and_src_bridge_port_to_meter_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE cfg_cpu_reason_and_src_bridge_port_to_cpu_rx_meter ( rdpa_cpu_reason       xi_cpu_reason,
                                                                                              BL_LILAC_RDD_CPU_METER_DTE           xi_cpu_meter,
                                                                                              BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  xi_src_port_mask )
{
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  bridge_port_vector;
    BL_LILAC_RDD_BRIDGE_PORT_DTE         bridge_port;

    for ( bridge_port_vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI; bridge_port_vector <= BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4; bridge_port_vector <<= 1 )
    {
        bridge_port = rdd_bridge_port_vector_to_bridge_port ( bridge_port_vector );

        if ( xi_src_port_mask & bridge_port_vector )
        {
            f_bl_lilac_rdd_cpu_reason_and_src_bridge_port_to_cpu_rx_meter_cfg ( xi_cpu_reason, xi_cpu_meter, bridge_port );
        }
        else
        {
            f_bl_lilac_rdd_cpu_reason_and_src_bridge_port_to_cpu_rx_meter_clear ( xi_cpu_reason, xi_cpu_meter, bridge_port );
        }
    }

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_bl_lilac_rdd_cpu_reason_to_cpu_rx_meter_ds ( rdpa_cpu_reason  xi_cpu_reason,
                                                                             BL_LILAC_RDD_CPU_METER_DTE      xi_cpu_meter )
{
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS  *cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS  *cpu_reason_to_meter_entry_ptr;

    cpu_reason_to_meter_table_ptr = ( RDD_DS_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_REASON_TO_METER_TABLE_ADDRESS );

    cpu_reason_to_meter_entry_ptr = &( cpu_reason_to_meter_table_ptr->entry[ xi_cpu_reason ] );
    
    RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( xi_cpu_meter, cpu_reason_to_meter_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_bl_lilac_rdd_cpu_reason_to_cpu_rx_meter_us ( rdpa_cpu_reason       xi_cpu_reason,
                                                                             BL_LILAC_RDD_CPU_METER_DTE           xi_cpu_meter,
                                                                             BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  xi_src_port_mask )
{
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS  *cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS  *cpu_reason_to_meter_entry_ptr;

    if ( ( xi_cpu_reason == rdpa_cpu_rx_reason_mcast ) || ( xi_cpu_reason == rdpa_cpu_rx_reason_bcast ) || ( xi_cpu_reason == rdpa_cpu_rx_reason_unknown_da ) )
    {
        cfg_cpu_reason_and_src_bridge_port_to_cpu_rx_meter ( xi_cpu_reason, xi_cpu_meter, xi_src_port_mask );
    }
    else
    {
        cpu_reason_to_meter_table_ptr = ( RDD_DS_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_REASON_TO_METER_TABLE_ADDRESS );

        cpu_reason_to_meter_entry_ptr = &( cpu_reason_to_meter_table_ptr->entry[ xi_cpu_reason ] );

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( xi_cpu_meter, cpu_reason_to_meter_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_reason_to_cpu_rx_meter ( rdpa_cpu_reason       xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_METER_DTE           xi_cpu_meter,
                                                        rdpa_traffic_dir                     xi_direction,
                                                        BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  xi_src_port_mask )
{
    /* check the validity of the input parameters - CPU-RX reason */
    if ( xi_cpu_reason >= _RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CPU_RX_REASON_ILLEGAL );
    }

    if ( xi_direction == rdpa_dir_ds )
    {
        f_bl_lilac_rdd_cpu_reason_to_cpu_rx_meter_ds ( xi_cpu_reason, xi_cpu_meter );
    }
    else
    {
        f_bl_lilac_rdd_cpu_reason_to_cpu_rx_meter_us ( xi_cpu_reason, xi_cpu_meter, xi_src_port_mask );
    }
    
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_meter_config ( BL_LILAC_RDD_CPU_METER_DTE  xi_cpu_meter,
                                                 uint16_t                    xi_average_rate,
                                                 uint16_t                    xi_burst_size,
                                                 rdpa_traffic_dir            xi_direction )
{
    RDD_CPU_RX_METER_TABLE_DTS  *cpu_meter_table_ptr;
    RDD_CPU_RX_METER_ENTRY_DTS  *cpu_meter_entry_ptr;
    static uint32_t             api_first_time_call[ 2 ] = { LILAC_RDD_TRUE, LILAC_RDD_TRUE };

    if ( xi_direction == rdpa_dir_ds )
    {
        cpu_meter_table_ptr = ( RDD_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_METER_TABLE_ADDRESS );
    }
    else
    {
        cpu_meter_table_ptr = ( RDD_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_METER_TABLE_ADDRESS );
    }

    cpu_meter_entry_ptr = &( cpu_meter_table_ptr->entry[ xi_cpu_meter ] );

    xi_burst_size = rdd_budget_to_alloc_unit ( xi_burst_size, LILAC_RDD_CPU_RX_METER_TIMER_PERIOD, 0 );

    RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_WRITE ( xi_burst_size, cpu_meter_entry_ptr );

    xi_average_rate = rdd_budget_to_alloc_unit ( xi_average_rate, LILAC_RDD_CPU_RX_METER_TIMER_PERIOD, 0 );

    RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_WRITE ( xi_average_rate, cpu_meter_entry_ptr );

    if ( api_first_time_call[ xi_direction ] )
    {
        rdd_timer_task_config ( xi_direction, LILAC_RDD_CPU_RX_METER_TIMER_PERIOD, CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID );
        api_first_time_call[ xi_direction ] = LILAC_RDD_FALSE;
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_meter_drop_counter_get (BL_LILAC_RDD_CPU_METER_DTE  xi_cpu_meter,
                                                           rdpa_traffic_dir            xi_direction,
                                                           uint32_t                    *xo_drop_counter)
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint16_t xo_drop_counter_val;

    rdd_error = rdd_2_bytes_counter_get ( CPU_RX_METERS_DROPPED_PACKETS_GROUP, xi_direction * BL_LILAC_RDD_CPU_METER_DISABLE + xi_cpu_meter, &xo_drop_counter_val );
    *xo_drop_counter = (uint32_t)xo_drop_counter_val;

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_write_eth_packet ( uint8_t                    *xi_packet_ptr,
                                                     uint32_t                   xi_packet_size,
                                                     BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                     uint8_t                    xi_wifi_ssid,
                                                     BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    uint8_t                    *packet_ddr_ptr;
#endif
    RDD_CPU_TX_DESCRIPTOR_DTS   *cpu_tx_descriptor_ptr;
    uint32_t                    cpu_tx_descriptor;
    uint32_t                    bpm_buffer_number;
    uint8_t                     cpu_tx_descriptor_valid;
    unsigned long               flags;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + g_cpu_tx_queue_write_ptr[ PICO_RUNNER_A ] );

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ ( cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr );

    if ( cpu_tx_descriptor_valid )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

#if !defined(FIRMWARE_INIT)
    if ( fi_bl_drv_bpm_req_buffer ( DRV_BPM_SP_SPARE_0, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size + g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8 ( packet_ddr_ptr, xi_packet_ptr, xi_packet_size );

    g_dummy_read = *( packet_ddr_ptr + xi_packet_size - 1 );
#else
    bpm_buffer_number = 0;
#endif

    /* write CPU-TX descriptor and validate it */
    cpu_tx_descriptor = 0;
    RDD_CPU_TX_DESCRIPTOR_CORE_PAYLOAD_OFFSET_L_WRITE(cpu_tx_descriptor, (g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET) / 2);
    
    RDD_CPU_TX_DESCRIPTOR_BPM_BUFFER_NUMBER_L_WRITE(cpu_tx_descriptor, bpm_buffer_number);
    /* !!! assuming this function is only used by 63138/63148 CFE, we hardcode 1 to lag_port_pti */
    RDD_CPU_TX_DESCRIPTOR_CORE_LAG_PORT_PTI_L_WRITE(cpu_tx_descriptor, 1);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor);

    cpu_tx_descriptor = 0;
    RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_WRITE(cpu_tx_descriptor, xi_packet_size + 4);
    RDD_CPU_TX_DESCRIPTOR_CORE_SRC_BRIDGE_PORT_L_WRITE(cpu_tx_descriptor, SPARE_0_SRC_PORT);
    RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_L_WRITE(cpu_tx_descriptor, xi_queue_id);
    RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_L_WRITE(cpu_tx_descriptor, xi_emac_id);
    RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, LILAC_RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET);
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_L_WRITE(cpu_tx_descriptor, LILAC_RDD_TRUE);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr, cpu_tx_descriptor);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ PICO_RUNNER_A ] += sizeof(RDD_CPU_TX_DESCRIPTOR_DTS);
    g_cpu_tx_queue_write_ptr[ PICO_RUNNER_A ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_PICO_THREAD_NUMBER >> 5;
    runner_cpu_wakeup_register.thread_num = CPU_TX_PICO_THREAD_NUMBER & 0x1f;
    runner_cpu_wakeup_register.urgent_req = LILAC_RDD_FALSE;

    RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_flow_pm_counters_get ( uint32_t                                xi_flow_id,
                                                  BL_LILAC_RDD_FLOW_PM_COUNTERS_TYPE_DTE  xi_flow_pm_counters_type,
                                                  bdmf_boolean                            xi_clear_counters,
                                                  BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE       *xo_pm_counters )
{
    BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE  *pm_counters_buffer_ptr;
    BL_LILAC_RDD_ERROR_DTE             rdd_error;
    unsigned long                      flags;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    /* read pm counters of a single port and reset its value */
    rdd_error = f_rdd_cpu_tx_send_message ( xi_flow_pm_counters_type, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, xi_flow_id, 0, 0, BL_LILAC_RDD_WAIT );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    pm_counters_buffer_ptr = ( BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + PM_COUNTERS_BUFFER_ADDRESS );

    xo_pm_counters->good_rx_packet           = swap4bytes (pm_counters_buffer_ptr->good_rx_packet);
    xo_pm_counters->good_rx_bytes            = swap4bytes (pm_counters_buffer_ptr->good_rx_bytes);
    xo_pm_counters->good_tx_packet           = swap4bytes (pm_counters_buffer_ptr->good_tx_packet);
    xo_pm_counters->good_tx_bytes            = swap4bytes (pm_counters_buffer_ptr->good_tx_bytes);
    xo_pm_counters->error_rx_packets_discard = swap2bytes (pm_counters_buffer_ptr->error_rx_packets_discard);
    xo_pm_counters->error_tx_packets_discard = swap2bytes (pm_counters_buffer_ptr->error_tx_packets_discard);

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_bridge_port_pm_counters_get ( BL_LILAC_RDD_BRIDGE_PORT_DTE              xi_bridge_port,
                                                         bdmf_boolean                              xi_clear_counters,
                                                         BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE  *xo_pm_counters )
{
    BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE  *pm_counters_buffer_ptr;
    BL_LILAC_RDD_ERROR_DTE                    rdd_error;
    unsigned long                             flags;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    /* read pm counters of a single port and reset its value */
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, xi_bridge_port, 0, 0, BL_LILAC_RDD_WAIT );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    pm_counters_buffer_ptr = ( BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + PM_COUNTERS_BUFFER_ADDRESS );

    if ( BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(xi_bridge_port) ) // DSL
    {
        xo_pm_counters->rx_valid = pm_counters_buffer_ptr->rx_valid;
        xo_pm_counters->tx_valid = pm_counters_buffer_ptr->tx_valid;
    }
    else if ( xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT )
    {
        xo_pm_counters->rx_valid = pm_counters_buffer_ptr->rx_valid;
        xo_pm_counters->tx_valid = 0;
    }
    else
    {
        xo_pm_counters->rx_valid = 0;
        xo_pm_counters->tx_valid = 0;
    }
    xo_pm_counters->error_rx_bpm_congestion   = pm_counters_buffer_ptr->error_rx_bpm_congestion;
    xo_pm_counters->bridge_filtered_packets   = pm_counters_buffer_ptr->bridge_filtered_packets;
    xo_pm_counters->bridge_tx_packets_discard = pm_counters_buffer_ptr->bridge_tx_packets_discard;

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

bdmf_error_t rdd_vport_pm_counters_get ( BL_LILAC_RDD_BRIDGE_PORT_DTE              xi_bridge_port,
                                         bdmf_boolean                              xi_clear_counters,
                                         BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE  *xo_pm_counters )
{
    return (bdmf_error_t) rdd_bridge_port_pm_counters_get(xi_bridge_port, xi_clear_counters, xo_pm_counters);
}


BL_LILAC_RDD_ERROR_DTE rdd_crc_error_counter_get ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                   bdmf_boolean                  xi_clear_counters,
                                                   uint16_t                      *xo_crc_counter )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT )
    {
        rdd_error = rdd_2_bytes_counter_get ( WAN_BRIDGE_PORT_GROUP, WAN_CRC_ERROR_IPTV_COUNTER_OFFSET, xo_crc_counter );
    }
    else
    {
        rdd_error = rdd_2_bytes_counter_get ( WAN_BRIDGE_PORT_GROUP, WAN_CRC_ERROR_NORMAL_COUNTER_OFFSET, xo_crc_counter );
    }

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_subnet_counters_get ( BL_LILAC_RDD_SUBNET_ID_DTE           xi_subnet_id,
                                                 BL_LILAC_RDD_BRIDGE_PORT_DTE         xi_subnet_port,
                                                 BL_LILAC_RDD_SUBNET_PM_COUNTERS_DTE  *xo_subnet_counters )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    int dropped_offset = SUBNET_DROPPED_PACKETS_SUB_GROUP_OFFSET ;

    if ( xi_subnet_id == BL_LILAC_RDD_SUBNET_BRIDGE )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_SUBNET_ID );
    }

    rdd_error = rdd_4_bytes_counter_get ( SUBNET_RX_GROUP, xi_subnet_id, &xo_subnet_counters->good_rx_packet );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_4_bytes_counter_get ( SUBNET_TX_GROUP, xi_subnet_id, &xo_subnet_counters->good_tx_packet );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_4_bytes_counter_get ( SUBNET_RX_BYTES_GROUP, xi_subnet_id, &xo_subnet_counters->good_rx_bytes );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_4_bytes_counter_get ( SUBNET_TX_BYTES_GROUP, xi_subnet_id, &xo_subnet_counters->good_tx_bytes );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    if (xi_subnet_port == BL_LILAC_RDD_WAN0_BRIDGE_PORT)
       dropped_offset += 1 ;

    rdd_error = rdd_2_bytes_counter_get ( SUBNET_RX_GROUP, dropped_offset + xi_subnet_id, &xo_subnet_counters->rx_dropped_packet );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( SUBNET_TX_GROUP, dropped_offset + xi_subnet_id, &xo_subnet_counters->tx_dropped_packet );

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_various_counters_get ( rdpa_traffic_dir                   xi_direction,
                                                  uint32_t                           xi_various_counters_mask,
                                                  bdmf_boolean                       xi_clear_counters,
                                                  BL_LILAC_RDD_VARIOUS_COUNTERS_DTE  *xo_various_counters )
{
    uint32_t                ingress_filter_idx;
    uint32_t                l4_filter_idx;
    uint32_t                counters_group;
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    if ( xi_direction == rdpa_dir_ds )
    {
        counters_group = DOWNSTREAM_VARIOUS_PACKETS_GROUP;
    }
    else
    {
        counters_group = UPSTREAM_VARIOUS_PACKETS_GROUP;
    }

    if ( xi_various_counters_mask & ETHERNET_FLOW_ACTION_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, ETHERNET_FLOW_DROP_ACTION_COUNTER_OFFSET, &xo_various_counters->eth_flow_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & VLAN_SWITCHING_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, VLAN_SWITCHING_DROP_COUNTER_OFFSET, &xo_various_counters->vlan_switching_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & SA_LOOKUP_FAILURE_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, SA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET, &xo_various_counters->sa_lookup_failure_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & DA_LOOKUP_FAILURE_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, DA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET, &xo_various_counters->da_lookup_failure_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & SA_ACTION_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, SA_ACTION_DROP_COUNTER_OFFSET, &xo_various_counters->sa_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & DA_ACTION_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, DA_ACTION_DROP_COUNTER_OFFSET, &xo_various_counters->da_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & FORWARDING_MATRIX_DISABLED_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, FORWARDING_MATRIX_DISABLED_DROP_COUNTER_OFFSET, &xo_various_counters->forwarding_matrix_disabled_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & CONNECTION_ACTION_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, CONNECTION_ACTION_DROP_COUNTER_OFFSET, &xo_various_counters->connection_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & TPID_DETECT_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, TPID_DETECT_DROP_COUNTER_OFFSET, &xo_various_counters->tpid_detect_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & INVALID_SUBNET_IP_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, INVALID_SUBNET_IP_DROP_COUNTER_OFFSET, &xo_various_counters->invalid_subnet_ip_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & INGRESS_FILTERS_DROP_COUNTER_MASK )
    {
        for ( ingress_filter_idx = 0; ingress_filter_idx < BL_LILAC_RDD_STOP_FILTER_NUMBER; ingress_filter_idx++ )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, INGRESS_FILTER_DROP_SUB_GROUP_OFFSET + ingress_filter_idx, &xo_various_counters->ingress_filters_drop[ ingress_filter_idx ] );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }
    }

    if ( xi_various_counters_mask & IP_VALIDATION_FILTER_DROP_COUNTER_MASK )
    {
        for ( ingress_filter_idx = 0; ingress_filter_idx < 2; ingress_filter_idx++ )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, INGRESS_FILTER_IP_VALIDATIOH_GROUP_OFFSET + ingress_filter_idx, &xo_various_counters->ip_validation_filter_drop[ ingress_filter_idx ] );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }
    }

    if ( xi_various_counters_mask & LAYER4_FILTERS_DROP_COUNTER_MASK )
    {
        for ( l4_filter_idx = 0; l4_filter_idx <= RDD_LAYER4_FILTER_UNKNOWN; l4_filter_idx++ )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, LAYER4_FILTER_DROP_SUB_GROUP_OFFSET + l4_filter_idx, &xo_various_counters->layer4_filters_drop[ l4_filter_idx ] );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }
    }

    if ( xi_direction == rdpa_dir_ds )
    {
        if ( xi_various_counters_mask & INVALID_LAYER2_PROTOCOL_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, INVALID_LAYER2_PROTOCOL_DROP_COUNTER_OFFSET, &xo_various_counters->invalid_layer2_protocol_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & FIREWALL_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, FIREWALL_DROP_COUNTER_OFFSET, &xo_various_counters->firewall_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DST_MAC_NON_ROUTER_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DST_MAC_NON_ROUTER_COUNTER_OFFSET, &xo_various_counters->dst_mac_non_router_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & EMAC_LOOPBACK_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, EMAC_LOOPBACK_DROP_COUNTER, &xo_various_counters->emac_loopback_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & IPTV_LAYER3_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, IPTV_LAYER3_DROP_COUNTER_OFFSET, &xo_various_counters->iptv_layer3_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DOWNSTREAM_POLICERS_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DOWNSTREAM_POLICERS_DROP_COUNTER_OFFSET, &xo_various_counters->downstream_policers_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_OFFSET, &xo_various_counters->dual_stack_lite_congestion_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        rdd_error = rdd_2_bytes_counter_get ( counters_group, DOWNSTREAM_PARALLEL_PROCESSING_NO_SLAVE_WAIT_OFFSET, &xo_various_counters->ds_parallel_processing_no_avialable_slave );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }

        rdd_error = rdd_2_bytes_counter_get ( counters_group, DOWNSTREAM_PARALLEL_PROCESSING_REORDER_WAIT_OFFSET, &xo_various_counters->ds_parallel_processing_reorder_slaves );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }

        if ( xi_various_counters_mask & ABSOLUTE_ADDRESS_LIST_OVERFLOW_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ABSOLUTE_ADDRESS_LIST_OVERFLOW_OFFSET, &xo_various_counters->absolute_address_list_overflow_drop );
            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }
    }
    else
    {
        if ( xi_various_counters_mask & ACL_OUI_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ACL_OUI_DROP_COUNTER_OFFSET, &xo_various_counters->acl_oui_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & ACL_L2_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ACL_LAYER2_DROP_COUNTER_OFFSET, &xo_various_counters->acl_l2_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & ACL_L3_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ACL_LAYER3_DROP_COUNTER_OFFSET, &xo_various_counters->acl_l3_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & LOCAL_SWITCHING_CONGESTION_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, LAN_ENQUEUE_CONGESTION_COUNTER_OFFSET, &xo_various_counters->local_switching_congestion );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & EPON_DDR_QUEUEU_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, EPON_DDR_QUEUES_COUNTER_OFFSET, &xo_various_counters->us_ddr_queue_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ring_init ( uint32_t  xi_ring_id,
                                       uint8_t unused0,
                                       rdd_phys_addr_t xi_ring_address,
                                       uint32_t  xi_number_of_entries,
                                       uint32_t  xi_size_of_entry,
                                       uint32_t  xi_interrupt_id,
                                       uint32_t unused1,
                                       bdmf_phys_addr_t unused2,
                                       uint8_t unused3
                                       )
{
    RDD_RING_DESCRIPTORS_TABLE_DTS  *ring_table_ptr;
    RDD_RING_DESCRIPTOR_DTS         *ring_descriptor_ptr;


    ring_table_ptr = ( RDD_RING_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RING_DESCRIPTORS_TABLE_ADDRESS );

    ring_descriptor_ptr = &( ring_table_ptr->entry[ xi_ring_id ] );

    RDD_RING_DESCRIPTOR_ENTRIES_COUNTER_WRITE ( 0, ring_descriptor_ptr );
    RDD_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE ( xi_size_of_entry, ring_descriptor_ptr );
    RDD_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE ( xi_number_of_entries, ring_descriptor_ptr );
    RDD_RING_DESCRIPTOR_INTERRUPT_ID_WRITE ( 1 << xi_interrupt_id, ring_descriptor_ptr );
    RDD_RING_DESCRIPTOR_RING_POINTER_WRITE ( xi_ring_address, ring_descriptor_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ring_destroy ( uint32_t  xi_ring_id )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error = BL_LILAC_RDD_OK;
    unsigned long           flags;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

#if !defined(FIRMWARE_INIT)
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_RING_DESTROY, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, xi_ring_id, 0, 0, BL_LILAC_RDD_WAIT );
#endif
    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_interrupt_coalescing_config( uint32_t xi_ring_id,
                                                               uint32_t xi_timeout_us,
                                                               uint32_t xi_max_packet_count )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET           runner_timer_target_register;
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;
#endif
    RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS *ic_table_ptr;
    RDD_INTERRUPT_COALESCING_CONFIG_DTS       *ic_entry_ptr;
    uint16_t                                  *ic_timer_period;
    static uint32_t                           api_first_time_call = LILAC_RDD_TRUE;

    if( xi_ring_id > RDD_INTERRUPT_COALESCING_CONFIG_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_CPU_RX_REASON_ILLEGAL );
    }

    ic_table_ptr = ( RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + INTERRUPT_COALESCING_CONFIG_TABLE_ADDRESS );
    ic_entry_ptr =  &( ic_table_ptr->entry[ xi_ring_id ] );

    RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_TIMEOUT_WRITE( xi_timeout_us, ic_entry_ptr );
    RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_MAX_PACKET_COUNT_WRITE( xi_max_packet_count, ic_entry_ptr );

    if ( api_first_time_call )
    {
        ic_timer_period = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + INTERRUPT_COALESCING_TIMER_PERIOD_ADDRESS );
#if defined(FIRMWARE_INIT)
        MWRITE_16( ic_timer_period, 100 );
#else
#ifdef RUNNER_FWTRACE
        MWRITE_16( ic_timer_period, (INTERRUPT_COALESCING_TIMER_PERIOD*(1000/TIMER_PERIOD_NS)) );
#else
        MWRITE_16( ic_timer_period, INTERRUPT_COALESCING_TIMER_PERIOD );
#endif
        RUNNER_REGS_0_CFG_TIMER_TARGET_READ ( runner_timer_target_register );
        runner_timer_target_register.timer_5_7 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_5_7_PICO_CORE_VALUE;
        RUNNER_REGS_0_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );

        /* activate the interrupt coalescing task */
        runner_cpu_wakeup_register.req_trgt = CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER % 32;
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif
        api_first_time_call = LILAC_RDD_FALSE;
    }

    return ( BL_LILAC_RDD_OK );
}

static void f_timer_7_fw_addrs_add (uint16_t xi_pico_a_addr, uint16_t xi_main_b_addr)
{
    uint16_t *ds_timer_7_primitive_table_ptr;
    uint16_t *us_timer_7_primitive_table_ptr;
    int i;

    ds_timer_7_primitive_table_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TIMER_7_SCHEDULER_PRIMITIVE_TABLE_ADDRESS );
    us_timer_7_primitive_table_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TIMER_7_SCHEDULER_PRIMITIVE_TABLE_ADDRESS );

    // Find an available entry to write the firmware timer routine start address
    for (i = 0; i < DS_TIMER_7_SCHEDULER_PRIMITIVE_TABLE_BYTE_SIZE / sizeof(uint16_t); i++)
    {
        if (!ds_timer_7_primitive_table_ptr[i])
        {

            MWRITE_16( &ds_timer_7_primitive_table_ptr[i], xi_pico_a_addr );
            MWRITE_16( &us_timer_7_primitive_table_ptr[i], xi_main_b_addr );
            break;
        }
    }
}

static void f_timer_7_initialize(void)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET           runner_timer_target_register;
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;
    uint16_t                               *timer_period;
#endif
    static uint32_t                        api_first_time_call = LILAC_RDD_TRUE;

    if ( api_first_time_call )
    {
#if !defined(FIRMWARE_INIT)
        timer_period = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + TIMER_7_TIMER_PERIOD_ADDRESS );
        MWRITE_16( timer_period, TIMER_7_TIMER_PERIOD );

        RUNNER_REGS_0_CFG_TIMER_TARGET_READ ( runner_timer_target_register );
        runner_timer_target_register.timer_5_7 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_5_7_PICO_CORE_VALUE;
        RUNNER_REGS_0_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );

        RUNNER_REGS_1_CFG_TIMER_TARGET_READ ( runner_timer_target_register );
        runner_timer_target_register.timer_5_7 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_5_7_MAIN_CORE_VALUE;
        RUNNER_REGS_1_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );

        /* activate the timer 7 tasks */
        runner_cpu_wakeup_register.req_trgt = US_TIMER_7_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = US_TIMER_7_THREAD_NUMBER % 32;
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );

        runner_cpu_wakeup_register.req_trgt = DS_TIMER_7_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = DS_TIMER_7_THREAD_NUMBER % 32;
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif
        api_first_time_call = LILAC_RDD_FALSE;
    }
}


#ifdef CONFIG_DHD_RUNNER
#if ( DHD_LLCSNAP_END_OFFSET != 22 )
#error "SPDSVC: DHD Ethernet Header size mismatch"
#endif
#endif

#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
bdmf_sysb  g_spdsvc_setup_sysb_ptr = (bdmf_sysb)(0xFFFFFFFF);
#endif

static inline uint32_t f_rdd_spdsvc_kbps_to_tokens(uint32_t xi_kbps)
{
    return ( uint32_t )( ( (1000/8) * xi_kbps ) / TIMER_7_TIMER_HZ );
}

static inline uint32_t f_rdd_spdsvc_mbs_to_bucket_size(uint32_t xi_mbs)
{
    uint32_t bucket_size = xi_mbs;

    if(bucket_size < SPDSVC_BUCKET_SIZE_MIN)
    {
        bucket_size = SPDSVC_BUCKET_SIZE_MIN;
    }

    return bucket_size;
}

static inline RDD_SPDSVC_CONTEXT_ENTRY_DTS *f_rdd_spdsvc_get_context_ptr( rdpa_traffic_dir xi_direction )
{
    if( xi_direction == rdpa_dir_us )
    {
        return ( RDD_SPDSVC_CONTEXT_ENTRY_DTS * )
            ( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_SPDSVC_CONTEXT_TABLE_ADDRESS );
    }
    else
    {
        return ( RDD_SPDSVC_CONTEXT_ENTRY_DTS * )
            ( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_SPDSVC_CONTEXT_TABLE_ADDRESS );
    }
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_spdsvc_config( uint32_t xi_kbps,
                                                   uint32_t xi_mbs,
                                                   uint32_t xi_copies,
                                                   uint32_t xi_total_length,
                                                   rdpa_traffic_dir xi_direction )
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( xi_direction );
    RDD_SPDSVC_CONTEXT_ENTRY_DTS spdsvc_context;

    RDD_SPDSVC_CONTEXT_ENTRY_TERMINATE_WRITE(0, spdsvc_context_ptr);

    RDD_SPDSVC_CONTEXT_ENTRY_BBH_DESCRIPTOR_0_WRITE( 0, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_BBH_DESCRIPTOR_1_WRITE( 0, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_SKB_FREE_INDEX_WRITE( 0xFFFF, spdsvc_context_ptr );

    spdsvc_context.tokens = f_rdd_spdsvc_kbps_to_tokens( xi_kbps );
    spdsvc_context.bucket_size = f_rdd_spdsvc_mbs_to_bucket_size( spdsvc_context.tokens + xi_mbs );

    RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_WRITE( 0, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_WRITE( xi_copies, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_LENGTH_WRITE( xi_total_length, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TOKENS_WRITE( spdsvc_context.tokens, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_BUCKET_SIZE_WRITE( spdsvc_context.bucket_size, spdsvc_context_ptr );

    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_DISCARDS_WRITE( 0, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_WRITES_WRITE( 0, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_READS_WRITE( 0, spdsvc_context_ptr );

#if defined(CC_RDD_CPU_SPDSVC_DEBUG) && !defined(FIRMWARE_INIT)
    __rdd_cpu_trace("\n%s: kbps %u (tokens %u), mbs %u (bucket_size %u), copies %u, total_length %u, direction %s\n\n",
                    __FUNCTION__, xi_kbps, spdsvc_context.tokens, xi_mbs, spdsvc_context.bucket_size, xi_copies, xi_total_length,
                    ( xi_direction == rdpa_dir_us ) ? "US" : "DS");
#endif
    return ( BL_LILAC_RDD_OK );
}

#define RDD_CPU_SPDSVC_IS_RUNNING(__copies_in_transit, __total_copies)  \
    ( (__copies_in_transit) || (__total_copies) )

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_config( uint32_t xi_kbps,
                                          uint32_t xi_mbs,
                                          uint32_t xi_copies,
                                          uint32_t xi_total_length )
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr;
    uint32_t copies_in_transit;
    uint32_t total_copies;
    int ret;

    spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_us );
    RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_READ( copies_in_transit, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_READ( total_copies, spdsvc_context_ptr );

    if( RDD_CPU_SPDSVC_IS_RUNNING(copies_in_transit, total_copies) )
    {
        spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_ds );
        RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_READ( copies_in_transit, spdsvc_context_ptr );
        RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_READ( total_copies, spdsvc_context_ptr );

        if( RDD_CPU_SPDSVC_IS_RUNNING(copies_in_transit, total_copies) )
        {
            return ( BL_LILAC_RDD_ERROR_SPDSVC_RESOURCE_BUSY );
        }
    }

    ret = f_rdd_spdsvc_config( xi_kbps, xi_mbs, xi_copies, xi_total_length, rdpa_dir_us );
    if( ret == BL_LILAC_RDD_OK )
    {
        ret = f_rdd_spdsvc_config( xi_kbps, xi_mbs, xi_copies, xi_total_length, rdpa_dir_ds );
    }

    return ret;
}

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_terminate( void )
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr;

    spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_us );
    RDD_SPDSVC_CONTEXT_ENTRY_TERMINATE_WRITE(1, spdsvc_context_ptr);

    spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_ds );
    RDD_SPDSVC_CONTEXT_ENTRY_TERMINATE_WRITE(1, spdsvc_context_ptr);

    return BL_LILAC_RDD_OK;
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_spdsvc_get_tx_result( uint8_t *xo_running_p,
                                                          uint32_t *xo_tx_packets_p,
                                                          uint32_t *xo_tx_discards_p,
                                                          rdpa_traffic_dir xi_direction )
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( xi_direction );
    RDD_SPDSVC_CONTEXT_ENTRY_DTS spdsvc_context;

    RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_READ( spdsvc_context.copies_in_transit, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_READ( spdsvc_context.total_copies, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_READS_READ( spdsvc_context.tx_queue_reads, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_DISCARDS_READ( spdsvc_context.tx_queue_discards, spdsvc_context_ptr );

    *xo_running_p &= RDD_CPU_SPDSVC_IS_RUNNING(spdsvc_context.copies_in_transit, spdsvc_context.total_copies) ? 1 : 0;
    *xo_tx_packets_p += spdsvc_context.tx_queue_reads;
    *xo_tx_discards_p += spdsvc_context.tx_queue_discards;

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_get_tx_result( uint8_t *xo_running_p,
                                                 uint32_t *xo_tx_packets_p,
                                                 uint32_t *xo_tx_discards_p )
{
    int ret;

    *xo_running_p = 1;
    *xo_tx_packets_p = 0;
    *xo_tx_discards_p = 0;

    ret = f_rdd_spdsvc_get_tx_result( xo_running_p, xo_tx_packets_p, xo_tx_discards_p, rdpa_dir_us );
    if( ret == BL_LILAC_RDD_OK )
    {
        ret = f_rdd_spdsvc_get_tx_result( xo_running_p, xo_tx_packets_p, xo_tx_discards_p, rdpa_dir_ds );
    }

    return ret;
}

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_get_rx_time( uint32_t *xo_rx_time_usec_p )
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr;
    uint32_t start_time_usec;
    uint32_t end_time_usec;

    spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_us );
    RDD_SPDSVC_CONTEXT_ENTRY_START_TIME_USEC_READ( start_time_usec, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_END_TIME_USEC_READ( end_time_usec, spdsvc_context_ptr );

    *xo_rx_time_usec_p = (end_time_usec - start_time_usec);

    spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_ds );
    RDD_SPDSVC_CONTEXT_ENTRY_START_TIME_USEC_READ( start_time_usec, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_END_TIME_USEC_READ( end_time_usec, spdsvc_context_ptr );

    *xo_rx_time_usec_p += (end_time_usec - start_time_usec);

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_reset_rx_time( void )
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr;

    spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_us );
    RDD_SPDSVC_CONTEXT_ENTRY_START_TIME_USEC_WRITE( 0, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_END_TIME_USEC_WRITE( 0, spdsvc_context_ptr );

    spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( rdpa_dir_ds );
    RDD_SPDSVC_CONTEXT_ENTRY_START_TIME_USEC_WRITE( 0, spdsvc_context_ptr );
    RDD_SPDSVC_CONTEXT_ENTRY_END_TIME_USEC_WRITE( 0, spdsvc_context_ptr );

    return ( BL_LILAC_RDD_OK );
}

#if defined(CONFIG_BCM_SPDSVC_SUPPORT) && !defined(RDD_BASIC)
static void f_rdd_spdsvc_initialize_structs( rdpa_traffic_dir xi_direction )
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr = f_rdd_spdsvc_get_context_ptr( xi_direction );

    memset( spdsvc_context_ptr, 0, sizeof( RDD_SPDSVC_CONTEXT_ENTRY_DTS ) );

#if defined(FIRMWARE_INIT)
    rdd_spdsvc_config( 100000, 2000, 10, 1514 );
#else
#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
    __rdd_cpu_trace("\n\tspdsvc_context_ptr %p\n\n", spdsvc_context_ptr);
#endif
    RDD_SPDSVC_CONTEXT_ENTRY_SKB_FREE_INDEX_WRITE( 0xFFFF, spdsvc_context_ptr );
#endif
}

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_initialize( void )
{
    static uint32_t                        api_first_time_call = LILAC_RDD_TRUE;

    f_timer_7_fw_addrs_add(runner_c_spdsvc_timer_wakeup_request, runner_b_spdsvc_timer_wakeup_request);
    f_timer_7_initialize();
    if ( api_first_time_call )
    {
        f_rdd_spdsvc_initialize_structs( rdpa_dir_us );
        f_rdd_spdsvc_initialize_structs( rdpa_dir_ds );
        api_first_time_call = LILAC_RDD_FALSE;
    }

    return ( BL_LILAC_RDD_OK );
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_free_skb_timer_config ( void )
{
    static uint32_t               api_first_time_call = LILAC_RDD_TRUE;

    if ( api_first_time_call )
    {
#if !defined(FIRMWARE_INIT)

        rdd_timer_task_config ( rdpa_dir_us, FREE_SKB_INDEX_TIMER_PERIOD, FREE_SKB_INDEX_ALLOCATE_CODE_ID );
        rdd_timer_task_config ( rdpa_dir_ds, FREE_SKB_INDEX_TIMER_PERIOD, FREE_SKB_INDEX_ALLOCATE_CODE_ID );
#else
        rdd_timer_task_config ( rdpa_dir_us, 100, FREE_SKB_INDEX_ALLOCATE_CODE_ID );
        rdd_timer_task_config ( rdpa_dir_ds, 100, FREE_SKB_INDEX_ALLOCATE_CODE_ID );
#endif

        api_first_time_call = LILAC_RDD_FALSE;
    }

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_cpu_total_pps_rate_limiter_config(uint32_t xi_rate_kpps)
{
    RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_DTS   *pps_rl_entry_ptr;
    uint32_t                               tokens;

    tokens = ((xi_rate_kpps * TIMER_7_TIMER_PERIOD) / 1000) + 1;

    pps_rl_entry_ptr = ( RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TOTAL_PPS_RATE_LIMITER_ADDRESS );
    RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_TOKENS_WRITE(tokens, pps_rl_entry_ptr);
    RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_BUCKET_SIZE_WRITE(tokens * 2, pps_rl_entry_ptr);
    RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_BUCKET_WRITE(0, pps_rl_entry_ptr);

    pps_rl_entry_ptr = ( RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TOTAL_PPS_RATE_LIMITER_ADDRESS );
    RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_TOKENS_WRITE(tokens, pps_rl_entry_ptr);
    RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_BUCKET_SIZE_WRITE(tokens * 2, pps_rl_entry_ptr);
    RDD_TOTAL_PPS_RATE_LIMITER_ENTRY_BUCKET_WRITE(0, pps_rl_entry_ptr);

    f_timer_7_fw_addrs_add(runner_c_pps_rate_limiter_timer_wakeup_request, runner_b_pps_rate_limiter_timer_wakeup_request);
    f_timer_7_initialize();

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_cso_counters_get ( RDD_CSO_COUNTERS_ENTRY_DTS *xo_cso_counters_ptr )
{
    RDD_CSO_CONTEXT_ENTRY_DTS   *cso_context_ptr;
    unsigned long               flags;

    cso_context_ptr = ( RDD_CSO_CONTEXT_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CSO_CONTEXT_TABLE_ADDRESS );

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    RDD_CSO_CONTEXT_ENTRY_GOOD_CSUM_PACKETS_READ( xo_cso_counters_ptr->good_csum_packets, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_NO_CSUM_PACKETS_READ( xo_cso_counters_ptr->no_csum_packets, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_BAD_IPV4_HDR_CSUM_PACKETS_READ( xo_cso_counters_ptr->bad_ipv4_hdr_csum_packets, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_BAD_TCP_UDP_CSUM_PACKETS_READ( xo_cso_counters_ptr->bad_tcp_udp_csum_packets, cso_context_ptr );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_cso_context_get ( RDD_CSO_CONTEXT_ENTRY_DTS *xo_cso_context_ptr )
{
    RDD_CSO_CONTEXT_ENTRY_DTS   *cso_context_ptr;
    unsigned long               flags;

    cso_context_ptr = ( RDD_CSO_CONTEXT_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CSO_CONTEXT_TABLE_ADDRESS );

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    RDD_CSO_CONTEXT_ENTRY_SUMMARY_READ( xo_cso_context_ptr->summary, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_PACKET_LENGTH_READ( xo_cso_context_ptr->packet_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_LINEAR_LENGTH_READ( xo_cso_context_ptr->linear_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_PACKET_HEADER_LENGTH_READ( xo_cso_context_ptr->packet_header_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_IP_PROTOCOL_READ( xo_cso_context_ptr->ip_protocol, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_IP_HEADER_OFFSET_READ( xo_cso_context_ptr->ip_header_offset, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_IP_HEADER_LENGTH_READ( xo_cso_context_ptr->ip_header_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_IP_TOTAL_LENGTH_READ( xo_cso_context_ptr->ip_total_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_IPV4_CSUM_READ( xo_cso_context_ptr->ipv4_csum, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_HEADER_OFFSET_READ( xo_cso_context_ptr->tcp_udp_header_offset, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_HEADER_LENGTH_READ( xo_cso_context_ptr->tcp_udp_header_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_TOTAL_LENGTH_READ( xo_cso_context_ptr->tcp_udp_total_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_CSUM_READ( xo_cso_context_ptr->tcp_udp_csum, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_MAX_CHUNK_LENGTH_READ( xo_cso_context_ptr->max_chunk_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_CHUNK_BYTES_LEFT_READ( xo_cso_context_ptr->chunk_bytes_left, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_NR_FRAGS_READ( xo_cso_context_ptr->nr_frags, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_FRAG_INDEX_READ( xo_cso_context_ptr->frag_index, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_FRAG_LEN_READ( xo_cso_context_ptr->frag_len, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_FRAG_DATA_READ( xo_cso_context_ptr->frag_data, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_GOOD_CSUM_PACKETS_READ( xo_cso_context_ptr->good_csum_packets, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_NO_CSUM_PACKETS_READ( xo_cso_context_ptr->no_csum_packets, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_BAD_IPV4_HDR_CSUM_PACKETS_READ( xo_cso_context_ptr->bad_ipv4_hdr_csum_packets, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_BAD_TCP_UDP_CSUM_PACKETS_READ( xo_cso_context_ptr->bad_tcp_udp_csum_packets, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_FAIL_CODE_READ( xo_cso_context_ptr->fail_code, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_DMA_SYNC_READ( xo_cso_context_ptr->dma_sync, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_SEG_LENGTH_READ( xo_cso_context_ptr->seg_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_SEG_BYTES_LEFT_READ( xo_cso_context_ptr->seg_bytes_left, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_PAYLOAD_LENGTH_READ( xo_cso_context_ptr->payload_length, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_PAYLOAD_BYTES_LEFT_READ( xo_cso_context_ptr->payload_bytes_left, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_PAYLOAD_PTR_READ( xo_cso_context_ptr->payload_ptr, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_DDR_PAYLOAD_OFFSET_READ( xo_cso_context_ptr->ddr_payload_offset, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_DDR_SRC_ADDRESS_READ( xo_cso_context_ptr->ddr_src_address, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_SAVED_IH_BUFFER_NUMBER_READ( xo_cso_context_ptr->saved_ih_buffer_number, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_SAVED_CSUM32_RET_ADDR_READ( xo_cso_context_ptr->saved_csum32_ret_addr, cso_context_ptr );
    RDD_CSO_CONTEXT_ENTRY_SAVED_R16_READ( xo_cso_context_ptr->saved_r16, cso_context_ptr );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

