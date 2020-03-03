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
#include "rdd_runner_defs_auto.h"

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
extern rdpa_bpm_buffer_size_t                  g_bpm_buffer_size;
#ifdef FIRMWARE_INIT
extern unsigned int CpuRxRingBase;
#endif

#ifdef CONFIG_DHD_RUNNER
extern bdmf_boolean is_dhd_enabled[];
#endif

#if !defined(RDD_BASIC)
#if !defined(FIRMWARE_INIT)
static inline int32_t f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port )
{
    switch ( xi_bridge_port )
    {
    case BL_LILAC_RDD_WAN_BRIDGE_PORT:

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
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS                   *ds_cpu_reason_to_meter_table_ptr;
    RDD_US_CPU_REASON_TO_METER_TABLE_DTS                   *us_cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS                      *cpu_reason_to_meter_entry_ptr;
    RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS  *cpu_reason_and_src_bridge_port_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS                      *cpu_reason_and_src_bridge_port_to_meter_entry_ptr;
    BL_LILAC_RDD_EMAC_ID_DTE                               bridge_port;
#if defined(FIRMWARE_INIT)
    RDD_CPU_RX_DESCRIPTOR_DTS                              *cpu_rx_descriptor_ptr;
    uint32_t                                               host_buffer_address;
#endif
    uint32_t                                               cpu_reason_per_port_index;
    uint16_t                                               *cpu_rx_ingress_queue_ptr;
    uint8_t                                                cpu_reason;
#if defined(FIRMWARE_INIT)
    uint32_t                                               i;

    /* Init Rings */
    cpu_rx_descriptor_ptr = ( RDD_CPU_RX_DESCRIPTOR_DTS * )CpuRxRingBase;

    host_buffer_address = SIMULATOR_DDR_RING_OFFSET + RDD_RING_DESCRIPTORS_TABLE_SIZE * SIMULATOR_DDR_RING_NUM_OF_ENTRIES * sizeof ( RDD_CPU_RX_DESCRIPTOR_DTS );

    for ( i = 0; i < RDD_RING_DESCRIPTORS_TABLE_SIZE * 10; i++, cpu_rx_descriptor_ptr++, host_buffer_address += g_bpm_buffer_size )
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

    cpu_rx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_RX_MIRRORING_PD_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( cpu_rx_ingress_queue_ptr, CPU_RX_MIRRORING_PD_INGRESS_QUEUE_ADDRESS );

    cpu_reason_to_cpu_rx_queue_table_ptr = ( RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS );
    
    /* set cpu reason direct_flow to queue_0 for backward compatibility*/ 
    cpu_reason_to_cpu_rx_queue_entry_ptr = &( cpu_reason_to_cpu_rx_queue_table_ptr->entry[ rdpa_cpu_rx_reason_direct_flow ] );
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE ( BL_LILAC_RDD_CPU_RX_QUEUE_0, cpu_reason_to_cpu_rx_queue_entry_ptr );

    ds_cpu_reason_to_meter_table_ptr = RDD_DS_CPU_REASON_TO_METER_TABLE_PTR();
    us_cpu_reason_to_meter_table_ptr = RDD_US_CPU_REASON_TO_METER_TABLE_PTR();

    for ( cpu_reason = rdpa_cpu_rx_reason_oam; cpu_reason < rdpa_cpu_reason__num_of; cpu_reason++ )
    {


        cpu_reason_to_meter_entry_ptr = &( ds_cpu_reason_to_meter_table_ptr->entry[ cpu_reason ] );

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( BL_LILAC_RDD_CPU_METER_DISABLE, cpu_reason_to_meter_entry_ptr );

        cpu_reason_to_meter_entry_ptr = &( us_cpu_reason_to_meter_table_ptr->entry[ cpu_reason ] );

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( BL_LILAC_RDD_CPU_METER_DISABLE, cpu_reason_to_meter_entry_ptr );

        if ( ( cpu_reason != rdpa_cpu_rx_reason_mcast ) && ( cpu_reason != rdpa_cpu_rx_reason_bcast ) && ( cpu_reason != rdpa_cpu_rx_reason_unknown_da ) )
        {
            continue;
        }

        cpu_reason_and_src_bridge_port_to_meter_table_ptr =
            ( RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_ADDRESS );

        cpu_reason_per_port_index = cpu_reason_to_cpu_per_port_reason_index(cpu_reason);

        for ( bridge_port = BL_LILAC_RDD_EMAC_ID_START; bridge_port < BL_LILAC_RDD_EMAC_ID_COUNT; bridge_port++ )
        {
            cpu_reason_and_src_bridge_port_to_meter_entry_ptr = &( cpu_reason_and_src_bridge_port_to_meter_table_ptr->entry[ cpu_reason_per_port_index ][ bridge_port ] );

            RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( BL_LILAC_RDD_CPU_METER_DISABLE, cpu_reason_and_src_bridge_port_to_meter_entry_ptr );
        }
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_initialize ( void )
{
    uint32_t  *ih_header_descriptor_ptr;
    uint32_t  ih_header_descriptor[ 2 ];
    uint16_t  *cpu_tx_ingress_queue_ptr;
    uint8_t   *wan_physical_port_ptr;
    uint32_t  *ih_buffer_bbh_ptr;
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

    g_cpu_tx_queue_write_ptr[ FAST_RUNNER_A ] = CPU_TX_FAST_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[ FAST_RUNNER_B ] = CPU_TX_FAST_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[ PICO_RUNNER_A ] = CPU_TX_PICO_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[ PICO_RUNNER_B ] = CPU_TX_PICO_QUEUE_ADDRESS;

    g_cpu_tx_queue_abs_data_ptr_write_ptr[ FAST_RUNNER_A ] = DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[ FAST_RUNNER_B ] = US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[ PICO_RUNNER_A ] = DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[ PICO_RUNNER_B ] = US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;

    cpu_tx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_TX_PICO_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( cpu_tx_ingress_queue_ptr, CPU_TX_EMAC_LOOPBACK_QUEUE_ADDRESS );

    cpu_tx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_TX_PICO_US_FLOODING_QUEUE_PTR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    MWRITE_16( cpu_tx_ingress_queue_ptr, CPU_TX_US_FLOODING_QUEUE_ADDRESS );

    ih_buffer_bbh_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + IH_BUFFER_BBH_POINTER_ADDRESS );

    MWRITE_32( ih_buffer_bbh_ptr, ( ( BBH_PERIPHERAL_IH << 16 ) | ( LILAC_RDD_IH_BUFFER_BBH_ADDRESS + LILAC_RDD_RUNNER_B_IH_BUFFER_BBH_OFFSET ) ) );

    wan_physical_port_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WAN_PHYSICAL_PORT_ADDRESS - sizeof(RUNNER_COMMON));

    switch (g_wan_physical_port)
    {
        case BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH4:
        {
            MWRITE_8(wan_physical_port_ptr, ETH4_SRC_PORT);
            break;
        }
        case BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0:
        {
            MWRITE_8(wan_physical_port_ptr, ETH0_SRC_PORT);
            break;
        }
        case BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH1:
        {
            MWRITE_8(wan_physical_port_ptr, ETH1_SRC_PORT);
            break;
        }
        case BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH2:
        {
            MWRITE_8(wan_physical_port_ptr, ETH2_SRC_PORT);
            break;
        }
        case BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH3:
        {
            MWRITE_8(wan_physical_port_ptr, ETH3_SRC_PORT);
            break;
        }
        default:
        {
            MWRITE_8(wan_physical_port_ptr, WAN_SRC_PORT);
            break;
        }
    }

#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    {
        uint16_t *gso_queue_ptr_ptr;

        gso_queue_ptr_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + GSO_PICO_QUEUE_PTR_ADDRESS );
        MWRITE_16( gso_queue_ptr_ptr, GSO_PICO_QUEUE_ADDRESS );
    }

    bdmf_gso_desc_pool_create(RUNNER_MAX_GSO_DESC);
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
    if ( is_dhd_enabled[xi_parameter_2 >> 14] && xi_runner_index == PICO_RUNNER_A && xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_DHD_MESSAGE )
    {
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DHD_MSG_TYPE_WRITE( xi_parameter_1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_FLOW_RING_ID_WRITE( (xi_parameter_2 & ~0xc000), cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_RADIO_IDX_WRITE( xi_parameter_2 >> 14, cpu_tx_descriptor_ptr );
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
        RDD_CPU_TX_DESCRIPTOR_COUNTER_CLEAR_WRITE ( xi_parameter_2, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_SEND_XON_FRAME )
    {
        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET ) 
    {
        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DESCRIPTOR_COUNTER_CLEAR_WRITE ( xi_parameter_2, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_PM_COUNTER_GET )
    {
        RDD_CPU_TX_DESCRIPTOR_GROUP_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DESCRIPTOR_COUNTER_WRITE ( xi_parameter_2, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_WRITE ( xi_parameter_3 & 0x1, cpu_tx_descriptor_ptr );
        RDD_CPU_TX_DESCRIPTOR_COUNTER_CLEAR_WRITE( ( xi_parameter_3 >>1 ) & 0x1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_IPTV_MAC_COUNTER_GET )
    {
        RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_ACTIVATE_TCONT )
    {
        RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
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

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_RING_DESTROY && xi_runner_index == PICO_RUNNER_A )
    {
        RDD_CPU_TX_DESCRIPTOR_QUEUE_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    if ( xi_msg_type == LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTER_SET )
    {
        RDD_CPU_TX_DESCRIPTOR_REGISTER_NUMBER_WRITE ( xi_parameter_1, cpu_tx_descriptor_ptr );
    }

    RDD_CPU_TX_MESSAGE_DESCRIPTOR_VALID_WRITE ( LILAC_RDD_TRUE, cpu_tx_descriptor_ptr );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ xi_runner_index ] += LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE;
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

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    RDD_RING_DESCRIPTOR_DROP_COUNTER_READ ( *xo_number_of_packets, ring_descriptor_ptr );
    RDD_RING_DESCRIPTOR_DROP_COUNTER_WRITE ( 0, ring_descriptor_ptr );

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_reason_to_cpu_rx_queue ( rdpa_cpu_reason  xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_RX_QUEUE_DTE   xi_queue_id,
                                                        rdpa_traffic_dir                xi_direction )
{
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS  *cpu_reason_to_cpu_rx_queue_table_ptr;
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS  *cpu_reason_to_cpu_rx_queue_entry_ptr;
    uint8_t                                   cpu_queue;

    /* check the validity of the input parameters - CPU-RX reason */
    if ( xi_cpu_reason >= RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE )
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

    cpu_reason_to_cpu_rx_queue_entry_ptr = &( cpu_reason_to_cpu_rx_queue_table_ptr->entry[ xi_cpu_reason ] );

    cpu_queue = xi_queue_id;

    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE ( cpu_queue, cpu_reason_to_cpu_rx_queue_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_bl_lilac_rdd_cpu_reason_and_src_bridge_port_to_cpu_rx_meter_cfg ( rdpa_cpu_reason  xi_cpu_reason,
                                                                                                  BL_LILAC_RDD_CPU_METER_DTE      xi_cpu_meter,
                                                                                                  BL_LILAC_RDD_EMAC_ID_DTE    xi_src_port )
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
                                                                                                    BL_LILAC_RDD_EMAC_ID_DTE    xi_src_port )
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
                                                                                              uint32_t                             xi_src_port_mask )
{
    uint32_t                    bridge_port_vector;
    BL_LILAC_RDD_EMAC_ID_DTE    bridge_port;

    for ( bridge_port = BL_LILAC_RDD_EMAC_ID_START ; bridge_port < BL_LILAC_RDD_EMAC_ID_COUNT ; bridge_port++ )
    {
        bridge_port_vector = RDD_EMAC_PORT_TO_VECTOR ( bridge_port, 0 );

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
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS  *ds_cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS  *cpu_reason_to_meter_entry_ptr;

    ds_cpu_reason_to_meter_table_ptr = RDD_DS_CPU_REASON_TO_METER_TABLE_PTR();

    cpu_reason_to_meter_entry_ptr = &( ds_cpu_reason_to_meter_table_ptr->entry[ xi_cpu_reason ] );
    
    RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( xi_cpu_meter, cpu_reason_to_meter_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_bl_lilac_rdd_cpu_reason_to_cpu_rx_meter_us ( rdpa_cpu_reason       xi_cpu_reason,
                                                                             BL_LILAC_RDD_CPU_METER_DTE           xi_cpu_meter,
                                                                             uint32_t                             xi_src_port_mask )
{
    RDD_US_CPU_REASON_TO_METER_TABLE_DTS  *us_cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS  *cpu_reason_to_meter_entry_ptr;

    if ( ( xi_cpu_reason == rdpa_cpu_rx_reason_mcast ) || ( xi_cpu_reason == rdpa_cpu_rx_reason_bcast ) || ( xi_cpu_reason == rdpa_cpu_rx_reason_unknown_da ) )
    {
        cfg_cpu_reason_and_src_bridge_port_to_cpu_rx_meter ( xi_cpu_reason, xi_cpu_meter, xi_src_port_mask );
    }
    else
    {
        us_cpu_reason_to_meter_table_ptr = RDD_US_CPU_REASON_TO_METER_TABLE_PTR();

        cpu_reason_to_meter_entry_ptr = &( us_cpu_reason_to_meter_table_ptr->entry[ xi_cpu_reason ] );

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE ( xi_cpu_meter, cpu_reason_to_meter_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_reason_to_cpu_rx_meter ( rdpa_cpu_reason       xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_METER_DTE           xi_cpu_meter,
                                                        rdpa_traffic_dir                     xi_direction,
                                                        uint32_t                             xi_src_port_mask )
{
    /* check the validity of the input parameters - CPU-RX reason */
    if ( xi_cpu_reason >= RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE )
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

    RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_WRITE ( xi_burst_size, cpu_meter_entry_ptr );
    RDD_CPU_RX_METER_ENTRY_CURRENT_BUDGET_WRITE ( xi_burst_size, cpu_meter_entry_ptr );

    xi_average_rate = rdd_budget_to_alloc_unit ( xi_average_rate, LILAC_RDD_CPU_RX_METER_TIMER_PERIOD, 0 );

    RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_WRITE ( xi_average_rate, cpu_meter_entry_ptr );

    if ( api_first_time_call[ xi_direction ] )
    {
        rdd_timer_task_config ( xi_direction, LILAC_RDD_CPU_RX_METER_TIMER_PERIOD, CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID );
        api_first_time_call[ xi_direction ] = LILAC_RDD_FALSE;
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_meter_drop_counter_get ( BL_LILAC_RDD_CPU_METER_DTE  xi_cpu_meter,
                                                           rdpa_traffic_dir            xi_direction,
                                                           uint32_t                    *xo_drop_counter )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint16_t xo_drop_counter_val;

    rdd_error = rdd_2_bytes_counter_get ( CPU_RX_METERS_DROPPED_PACKETS_GROUP, xi_direction * BL_LILAC_RDD_CPU_METER_DISABLE + xi_cpu_meter, LILAC_RDD_TRUE, &xo_drop_counter_val);
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

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + g_cpu_tx_queue_write_ptr[ 2 ] );

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ ( cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr );

    if ( cpu_tx_descriptor_valid )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

#if !defined(FIRMWARE_INIT)
    if ( fi_bl_drv_bpm_req_buffer ( DRV_BPM_SP_SPARE_0, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
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
    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_WRITE ( ( g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET ) / 2 ) |
                        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE ( bpm_buffer_number ) |
                        RDD_CPU_TX_DESCRIPTOR_1588_INDICATION_L_WRITE ( 0 );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor );

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE ( LILAC_RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET ) |
#ifndef G9991
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE ( xi_packet_size + 4 ) |
#else
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE ( xi_packet_size ) |
#endif
                        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE ( SPARE_0_SRC_PORT ) |
                        RDD_CPU_TX_DESCRIPTOR_EMAC_L_WRITE ( xi_emac_id ) |
                        RDD_CPU_TX_DESCRIPTOR_QUEUE_L_WRITE ( xi_queue_id ) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE ( LILAC_RDD_TRUE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr, cpu_tx_descriptor );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ 2 ] += LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[ 2 ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_PICO_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_PICO_THREAD_NUMBER % 32;
    runner_cpu_wakeup_register.urgent_req = LILAC_RDD_FALSE;

    RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


#if !defined(RDD_BASIC)
BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_write_gpon_packet ( uint8_t                              *xi_packet_ptr,
                                                      uint32_t                             xi_packet_size,
                                                      uint32_t                             xi_upstream_gem_flow,
                                                      RDD_WAN_CHANNEL_ID                   xi_wan_channel,
                                                      BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller,
                                                      BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue,
                                                      uint8_t                              xi_exclusive_packet )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_CPU_WAKEUP     runner_cpu_wakeup_register;
    uint8_t                       *packet_ddr_ptr;
#endif
    RDD_CPU_TX_DESCRIPTOR_DTS      *cpu_tx_descriptor_ptr;
    RDD_WAN_TX_POINTERS_ENTRY_DTS  *wan_tx_pointers_entry_ptr;
    uint32_t                       cpu_tx_descriptor;
    uint32_t                       bpm_buffer_number;
    uint8_t                        cpu_tx_descriptor_valid;
    unsigned long                  flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    wan_tx_pointers_entry_ptr = &( wan_tx_pointers_table_ptr->entry[ xi_wan_channel ][ xi_rate_controller ][ xi_queue ] );

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + g_cpu_tx_queue_write_ptr[ 1 ] );

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ ( cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr );

    if ( cpu_tx_descriptor_valid )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

#if !defined(FIRMWARE_INIT)
    if ( fi_bl_drv_bpm_req_buffer ( DRV_BPM_SP_EMAC0, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
    {
        if ( xi_exclusive_packet ) 
        {
            if ( fi_bl_drv_bpm_req_buffer ( DRV_BPM_SP_SPARE_0, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
            {
                f_rdd_unlock_irq ( &int_lock_irq, flags );
                return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
            }
        }
        else
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
        }
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size + g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8 ( packet_ddr_ptr, xi_packet_ptr, xi_packet_size );

    g_dummy_read = *( packet_ddr_ptr + xi_packet_size - 1 );
#else
    bpm_buffer_number = 0;
#endif

    /* write CPU-TX descriptor and validate it */
    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_L_WRITE ( ( ( wan_tx_pointers_entry_ptr->wan_tx_queue_ptr - WAN_TX_QUEUES_TABLE_ADDRESS ) / sizeof ( RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS ) ) ) |
                        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE ( bpm_buffer_number ) |
                        RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_WRITE ( ( g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET ) / 2 );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor );

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE ( LILAC_RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET ) |
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE ( xi_packet_size + 4 ) |
                        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE ( SPARE_0_SRC_PORT ) |
                        RDD_CPU_TX_DESCRIPTOR_US_GEM_FLOW_L_WRITE ( xi_upstream_gem_flow ) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE ( LILAC_RDD_TRUE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr, cpu_tx_descriptor );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ 1 ] += LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[ 1 ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_FAST_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_FAST_THREAD_NUMBER % 32;
    runner_cpu_wakeup_register.urgent_req = LILAC_RDD_FALSE;

    RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_send_packet_to_wan_bridge ( uint8_t                       *xi_packet_ptr,
                                                              uint32_t                      xi_packet_size,
                                                              BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_src_bridge_port,
                                                              uint8_t                       xi_wifi_ssid,
															  uint8_t                       xi_exclusive_packet )
{
    RDD_CPU_TX_DESCRIPTOR_DTS   *cpu_tx_descriptor_ptr;
    uint32_t                    cpu_tx_descriptor;
    uint32_t                    bpm_buffer_number;
    uint32_t                    ih_class;
    uint8_t                     cpu_tx_descriptor_valid;
#if !defined(FIRMWARE_INIT)
    DRV_BPM_SP_USR              bpm_src_port;
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    uint8_t                    *packet_ddr_ptr;
#endif
    unsigned long               flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + g_cpu_tx_queue_write_ptr[ 3 ] );

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ ( cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr );

    if ( cpu_tx_descriptor_valid )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

#if !defined(FIRMWARE_INIT)

    if ( g_broadcom_switch_mode && ( xi_src_bridge_port >= BL_LILAC_RDD_LAN0_BRIDGE_PORT ) && ( xi_src_bridge_port <= BL_LILAC_RDD_LAN4_BRIDGE_PORT ) )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( g_broadcom_switch_physical_port );
    }
    else
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( xi_src_bridge_port );
    }

    if ( fi_bl_drv_bpm_req_buffer ( bpm_src_port, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
    {
        if ( xi_exclusive_packet )
        {
            if ( fi_bl_drv_bpm_req_buffer ( DRV_BPM_SP_SPARE_0, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
            {
                f_rdd_unlock_irq ( &int_lock_irq, flags );
                return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
            }
        }
        else
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
        }
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size +
                     g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8 ( packet_ddr_ptr, xi_packet_ptr, xi_packet_size );

    g_dummy_read = *( packet_ddr_ptr + xi_packet_size - 1 );
#else
    bpm_buffer_number = 0;
#endif

    ih_class = f_rdd_bridge_port_to_class_id ( xi_src_bridge_port );

    /* write CPU-TX descriptor */
    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_IH_CLASS_L_WRITE ( ih_class ) |
                        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE ( bpm_buffer_number ) |
                        RDD_CPU_TX_DESCRIPTOR_SSID_L_WRITE ( xi_wifi_ssid ) |
                        RDD_CPU_TX_DESCRIPTOR_ABS_FLAG_L_WRITE ( LILAC_RDD_FALSE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor );

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE ( LILAC_RDD_CPU_TX_COMMAND_BRIDGE_PACKET ) |
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE ( xi_packet_size + 4 ) |
                        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE ( xi_src_bridge_port ) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE ( LILAC_RDD_TRUE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr, cpu_tx_descriptor );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ 3 ] += LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[ 3 ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_PICO_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_PICO_THREAD_NUMBER % 32;

    RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_send_packet_to_lan_bridge ( uint8_t   *xi_packet_ptr,
                                                              uint32_t  xi_packet_size,
                                                              uint32_t  xi_downstream_gem_flow )
{
    RDD_CPU_TX_DESCRIPTOR_DTS   *cpu_tx_descriptor_ptr;
    uint32_t                    cpu_tx_descriptor;
    uint32_t                    bpm_buffer_number;
    uint32_t                    ih_class;
    uint8_t                     cpu_tx_descriptor_valid;
#if !defined(FIRMWARE_INIT)
    DRV_BPM_SP_USR              bpm_src_port;
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    uint8_t                    *packet_ddr_ptr;
#endif
    unsigned long               flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + g_cpu_tx_queue_write_ptr[ 0 ] );

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ ( cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr );

    if ( cpu_tx_descriptor_valid )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

#if !defined(FIRMWARE_INIT)

    if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_FIBER ) 
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_WAN_BRIDGE_PORT );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH4 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN4_BRIDGE_PORT );
    }
	else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0 )
	{
		bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN0_BRIDGE_PORT );
	}
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH1 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN1_BRIDGE_PORT );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH2 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN2_BRIDGE_PORT );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH3 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN3_BRIDGE_PORT );
    }
	else
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_NOT_ALLOWED );
    }

    if ( fi_bl_drv_bpm_req_buffer ( bpm_src_port, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size +
                     g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8 ( packet_ddr_ptr, xi_packet_ptr, xi_packet_size );

    g_dummy_read = *( packet_ddr_ptr + xi_packet_size - 1 );
#else
    bpm_buffer_number = 0;
#endif

    ih_class = f_rdd_bridge_port_to_class_id ( BL_LILAC_RDD_WAN_BRIDGE_PORT );

    /* write CPU-TX descriptor */
    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_IH_CLASS_L_WRITE ( ih_class ) |
                        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE ( bpm_buffer_number ) |
                        RDD_CPU_TX_DESCRIPTOR_ABS_FLAG_L_WRITE ( LILAC_RDD_FALSE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor );

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE ( LILAC_RDD_CPU_TX_COMMAND_BRIDGE_PACKET ) |
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE ( xi_packet_size + 4 ) |
                        RDD_CPU_TX_DESCRIPTOR_DS_GEM_FLOW_L_WRITE ( xi_downstream_gem_flow ) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE ( LILAC_RDD_TRUE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr, cpu_tx_descriptor );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ 0 ] += LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[ 0 ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_FAST_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_FAST_THREAD_NUMBER % 32;

    RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_send_packet_to_wan_interworking ( uint8_t                       *xi_packet_ptr,
                                                                    uint32_t                      xi_packet_size,
                                                                    BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_src_bridge_port,
                                                                    uint8_t                       xi_wifi_ssid,
                                                                    BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
															        uint8_t                       xi_exclusive_packet )
{
    RDD_CPU_TX_DESCRIPTOR_DTS   *cpu_tx_descriptor_ptr;
    uint32_t                    cpu_tx_descriptor;
    uint32_t                    bpm_buffer_number;
    uint32_t                    ih_class;
    uint8_t                     cpu_tx_descriptor_valid;
#if !defined(FIRMWARE_INIT)
    DRV_BPM_SP_USR              bpm_src_port;
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    uint8_t                    *packet_ddr_ptr;
#endif
    unsigned long               flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + g_cpu_tx_queue_write_ptr[ 3 ] );

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ ( cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr );

    if ( cpu_tx_descriptor_valid )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

#if !defined(FIRMWARE_INIT)

    bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( xi_src_bridge_port );

    if ( fi_bl_drv_bpm_req_buffer ( bpm_src_port, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
    {
        if ( xi_exclusive_packet )
        {
            if ( fi_bl_drv_bpm_req_buffer ( DRV_BPM_SP_SPARE_0, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
            {
                f_rdd_unlock_irq ( &int_lock_irq, flags );
                return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
            }
        }
        else
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
        }
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size +
                     g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8 ( packet_ddr_ptr, xi_packet_ptr, xi_packet_size );

    g_dummy_read = *( packet_ddr_ptr + xi_packet_size - 1 );
#else
    bpm_buffer_number = 0;
#endif

    ih_class = f_rdd_bridge_port_to_class_id ( xi_src_bridge_port );

    /* write CPU-TX descriptor */
    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_IH_CLASS_L_WRITE ( ih_class ) |
                        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE ( bpm_buffer_number ) |
                        RDD_CPU_TX_DESCRIPTOR_SSID_L_WRITE ( xi_wifi_ssid ) |
                        RDD_CPU_TX_DESCRIPTOR_ABS_FLAG_L_WRITE ( LILAC_RDD_FALSE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor );

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE ( LILAC_RDD_CPU_TX_COMMAND_INTERWORKING_PACKET ) |
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE ( xi_packet_size + 4 ) |
                        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE ( xi_src_bridge_port ) |
                        RDD_CPU_TX_DESCRIPTOR_SUBNET_ID_L_WRITE ( xi_subnet_id ) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE ( LILAC_RDD_TRUE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr, cpu_tx_descriptor );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ 3 ] += LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[ 3 ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_PICO_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_PICO_THREAD_NUMBER % 32;

    RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_send_packet_to_lan_interworking ( uint8_t                   *xi_packet_ptr,
                                                                    uint32_t                  xi_packet_size,
                                                                    uint32_t                  xi_downstream_gem_flow,
                                                                    BL_LILAC_RDD_EMAC_ID_DTE  xi_emac_id,
                                                                    uint8_t                   xi_wifi_ssid )
{
    RDD_CPU_TX_DESCRIPTOR_DTS   *cpu_tx_descriptor_ptr;
    uint32_t                    cpu_tx_descriptor;
    uint32_t                    bpm_buffer_number;
    uint32_t                    ih_class;
    uint8_t                     cpu_tx_descriptor_valid;
#if !defined(FIRMWARE_INIT)
    DRV_BPM_SP_USR              bpm_src_port;
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    uint8_t                    *packet_ddr_ptr;
#endif
    unsigned long               flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    cpu_tx_descriptor_ptr = ( RDD_CPU_TX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + g_cpu_tx_queue_write_ptr[ 0 ] );

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ ( cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr );

    if ( cpu_tx_descriptor_valid )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL );
    }

#if !defined(FIRMWARE_INIT)

    if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_FIBER ) 
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_WAN_BRIDGE_PORT );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH4 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN4_BRIDGE_PORT );
    }
	else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0 )
	{
		bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN0_BRIDGE_PORT );
	}
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH1 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN1_BRIDGE_PORT );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH2 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN2_BRIDGE_PORT );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH3 )
    {
        bpm_src_port = f_rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_LAN3_BRIDGE_PORT );
    }
	else
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_CPU_TX_NOT_ALLOWED );
    }

    if ( fi_bl_drv_bpm_req_buffer ( bpm_src_port, ( uint32_t * )&bpm_buffer_number ) != DRV_BPM_ERROR_NO_ERROR )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL );
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size +
                     g_ddr_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8 ( packet_ddr_ptr, xi_packet_ptr, xi_packet_size );

    g_dummy_read = *( packet_ddr_ptr + xi_packet_size - 1 );
#else
    bpm_buffer_number = 0;
#endif

    ih_class = f_rdd_bridge_port_to_class_id ( BL_LILAC_RDD_WAN_BRIDGE_PORT );

    /* write CPU-TX descriptor */
    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_IH_CLASS_L_WRITE ( ih_class ) |
                        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE ( bpm_buffer_number ) |
                        RDD_CPU_TX_DESCRIPTOR_SSID_L_WRITE ( xi_wifi_ssid ) |
                        RDD_CPU_TX_DESCRIPTOR_ABS_FLAG_L_WRITE ( LILAC_RDD_FALSE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor );

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE ( LILAC_RDD_CPU_TX_COMMAND_INTERWORKING_PACKET ) |
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE ( xi_packet_size + 4 ) |
                        RDD_CPU_TX_DESCRIPTOR_EMAC_L_WRITE ( xi_emac_id ) |
                        RDD_CPU_TX_DESCRIPTOR_DS_GEM_FLOW_L_WRITE ( xi_downstream_gem_flow ) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE ( LILAC_RDD_TRUE );

    MWRITE_32( ( uint8_t * )cpu_tx_descriptor_ptr, cpu_tx_descriptor );

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[ 0 ] += LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[ 0 ] &= LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_FAST_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_FAST_THREAD_NUMBER % 32;

    RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE bl_lilac_rdd_cpu_tx_get_released_skb_counter ( uint32_t  *xo_skb_counter )
{
    *xo_skb_counter = g_cpu_tx_released_skb_counter;
    return ( BL_LILAC_RDD_OK );
}


#endif /* !defined(RDD_BASIC) */

BL_LILAC_RDD_ERROR_DTE rdd_flow_pm_counters_get ( uint32_t                                xi_flow_id,
                                                  BL_LILAC_RDD_FLOW_PM_COUNTERS_TYPE_DTE  xi_flow_pm_counters_type,
                                                  bdmf_boolean                            xi_clear_counters,
                                                  BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE       *xo_pm_counters )
{
    BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE  *pm_counters_buffer_ptr;
    BL_LILAC_RDD_ERROR_DTE             rdd_error;
    unsigned long                      flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    /* read pm counters of a single port */
    rdd_error = f_rdd_cpu_tx_send_message ( xi_flow_pm_counters_type, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, xi_flow_id, xi_clear_counters, 0, BL_LILAC_RDD_WAIT );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    pm_counters_buffer_ptr = ( BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + PM_COUNTERS_BUFFER_ADDRESS );

    xo_pm_counters->good_rx_packet           = pm_counters_buffer_ptr->good_rx_packet;
    xo_pm_counters->good_rx_bytes            = pm_counters_buffer_ptr->good_rx_bytes;
    xo_pm_counters->good_tx_packet           = pm_counters_buffer_ptr->good_tx_packet;
    xo_pm_counters->good_tx_bytes            = pm_counters_buffer_ptr->good_tx_bytes;

    xo_pm_counters->error_rx_packets_discard = pm_counters_buffer_ptr->error_rx_packets_discard;
    xo_pm_counters->error_tx_packets_discard = pm_counters_buffer_ptr->error_tx_packets_discard;

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_bridge_port_pm_counters_get ( BL_LILAC_RDD_BRIDGE_PORT_DTE              xi_bridge_port,
                                                         bdmf_boolean                              xi_clear_counters,
                                                         BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE  *xo_pm_counters )
{
    BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE  *pm_counters_buffer_ptr;
    BL_LILAC_RDD_ERROR_DTE                    rdd_error;
    unsigned long                             flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    /* read pm counters of a single port and reset its value */
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, xi_bridge_port, xi_clear_counters, 0, BL_LILAC_RDD_WAIT );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    pm_counters_buffer_ptr = ( BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + PM_COUNTERS_BUFFER_ADDRESS );

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT )
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

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_crc_error_counter_get ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                   bdmf_boolean                  xi_clear_counters,
                                                   uint16_t                      *xo_crc_counter )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT )
    {
        rdd_error = rdd_2_bytes_counter_get ( WAN_BRIDGE_PORT_GROUP, WAN_CRC_ERROR_IPTV_COUNTER_OFFSET, xi_clear_counters, xo_crc_counter );
    }
    else
    {
        rdd_error = rdd_2_bytes_counter_get ( WAN_BRIDGE_PORT_GROUP, WAN_CRC_ERROR_NORMAL_COUNTER_OFFSET, xi_clear_counters, xo_crc_counter );
    }

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_various_counters_get ( rdpa_traffic_dir                   xi_direction,
                                                  uint32_t          				 xi_various_counters_mask,
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
        rdd_error = rdd_2_bytes_counter_get ( counters_group, ETHERNET_FLOW_DROP_ACTION_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->eth_flow_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & VLAN_SWITCHING_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, VLAN_SWITCHING_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->vlan_switching_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & SA_LOOKUP_FAILURE_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, SA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->sa_lookup_failure_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & DA_LOOKUP_FAILURE_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, DA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->da_lookup_failure_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & SA_ACTION_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, SA_ACTION_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->sa_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & DA_ACTION_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, DA_ACTION_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->da_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & FORWARDING_MATRIX_DISABLED_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, FORWARDING_MATRIX_DISABLED_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->forwarding_matrix_disabled_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & CONNECTION_ACTION_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, CONNECTION_ACTION_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->connection_action_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & TPID_DETECT_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, TPID_DETECT_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->tpid_detect_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & INVALID_SUBNET_IP_DROP_COUNTER_MASK )
    {
        rdd_error = rdd_2_bytes_counter_get ( counters_group, INVALID_SUBNET_IP_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->invalid_subnet_ip_drop );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    if ( xi_various_counters_mask & INGRESS_FILTERS_DROP_COUNTER_MASK )
    {
        for ( ingress_filter_idx = 0; ingress_filter_idx < BL_LILAC_RDD_INGRESS_FILTERS_NUMBER; ingress_filter_idx++ )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, INGRESS_FILTER_DROP_SUB_GROUP_OFFSET + ingress_filter_idx, xi_clear_counters, &xo_various_counters->ingress_filters_drop[ ingress_filter_idx ] );

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
            rdd_error = rdd_2_bytes_counter_get ( counters_group, INGRESS_FILTER_IP_VALIDATIOH_GROUP_OFFSET + ingress_filter_idx, xi_clear_counters, &xo_various_counters->ip_validation_filter_drop[ ingress_filter_idx ] );

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
            rdd_error = rdd_2_bytes_counter_get ( counters_group, LAYER4_FILTER_DROP_SUB_GROUP_OFFSET + l4_filter_idx, xi_clear_counters, &xo_various_counters->layer4_filters_drop[ l4_filter_idx ] );

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
            rdd_error = rdd_2_bytes_counter_get ( counters_group, INVALID_LAYER2_PROTOCOL_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->invalid_layer2_protocol_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & FIREWALL_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, FIREWALL_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->firewall_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DST_MAC_NON_ROUTER_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DST_MAC_NON_ROUTER_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->dst_mac_non_router_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & EMAC_LOOPBACK_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, EMAC_LOOPBACK_DROP_COUNTER, xi_clear_counters, &xo_various_counters->emac_loopback_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & IPTV_LAYER3_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, IPTV_LAYER3_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->iptv_layer3_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DOWNSTREAM_POLICERS_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DOWNSTREAM_POLICERS_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->downstream_policers_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->dual_stack_lite_congestion_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        rdd_error = rdd_2_bytes_counter_get ( counters_group, DOWNSTREAM_PARALLEL_PROCESSING_NO_SLAVE_WAIT_OFFSET, xi_clear_counters, &xo_various_counters->ds_parallel_processing_no_avialable_slave );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }

        rdd_error = rdd_2_bytes_counter_get ( counters_group, DOWNSTREAM_PARALLEL_PROCESSING_REORDER_WAIT_OFFSET, xi_clear_counters, &xo_various_counters->ds_parallel_processing_reorder_slaves );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }

        if ( xi_various_counters_mask & ABSOLUTE_ADDRESS_LIST_OVERFLOW_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ABSOLUTE_ADDRESS_LIST_OVERFLOW_OFFSET, xi_clear_counters, &xo_various_counters->absolute_address_list_overflow_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & WLAN_MCAST_COPY_FAILED_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, WLAN_MCAST_COPY_FAILED_OFFSET, xi_clear_counters, &xo_various_counters->wlan_mcast_copy_failed_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & WLAN_MCAST_OVERFLOW_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, WLAN_MCAST_OVERFLOW_OFFSET, xi_clear_counters, &xo_various_counters->wlan_mcast_overflow_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & WLAN_MCAST_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, WLAN_MCAST_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->wlan_mcast_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        rdd_error = rdd_2_bytes_counter_get ( counters_group, SBPM_ALLOC_NACK_REPLY_OFFSET, xi_clear_counters, &xo_various_counters->sbpm_alloc_reply_nack );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }
    else
    {
        if ( xi_various_counters_mask & ACL_OUI_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ACL_OUI_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->acl_oui_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & ACL_L2_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ACL_LAYER2_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->acl_l2_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & ACL_L3_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, ACL_LAYER3_DROP_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->acl_l3_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & LOCAL_SWITCHING_CONGESTION_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, LAN_ENQUEUE_CONGESTION_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->local_switching_congestion );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & EPON_DDR_QUEUEU_DROP_COUNTER_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, EPON_DDR_QUEUES_COUNTER_OFFSET, xi_clear_counters, &xo_various_counters->us_ddr_queue_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DHD_IH_CONGESTION_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DHD_IH_CONGESTION_OFFSET, xi_clear_counters, &xo_various_counters->dhd_ih_congestion_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }

        if ( xi_various_counters_mask & DHD_MALLOC_FAILED_MASK )
        {
            rdd_error = rdd_2_bytes_counter_get ( counters_group, DHD_MALLOC_FAILED_OFFSET, xi_clear_counters, &xo_various_counters->dhd_malloc_failed_drop );

            if ( rdd_error != BL_LILAC_RDD_OK )
            {
                return ( rdd_error );
            }
        }
    }

    return ( BL_LILAC_RDD_OK );
}

#ifdef G9991
BL_LILAC_RDD_ERROR_DTE rdd_g9991_pm_flow_counters_get ( uint32_t                               xi_flow_id,
                                                        bdmf_boolean                           xi_clear_counters,
                                                        RDD_G9991_PM_FLOW_COUNTERS_DTE         *xi_g9991_flow_counters)
{
    BL_LILAC_RDD_ERROR_DTE rdd_error;
#define G9991_LAST_DATA_SID 23

    if (xi_flow_id < 0 || xi_flow_id > G9991_LAST_DATA_SID)
        return ( BL_LILAC_RDD_OK );

    rdd_error = rdd_4_bytes_counter_get ( DOWNSTREAM_VALID_PACKETS_GROUP_G9991, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->tx_packets );
    rdd_error = rdd_error ? : rdd_4_bytes_counter_get ( DOWNSTREAM_VALID_PACKETS_GROUP_G9991_MC, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->tx_mcast_packets );
    rdd_error = rdd_error ? : rdd_4_bytes_counter_get ( DOWNSTREAM_VALID_PACKETS_GROUP_G9991_BC, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->tx_bcast_packets );
    rdd_error = rdd_error ? : rdd_4_bytes_counter_get ( DOWNSTREAM_VALID_BYTES_GROUP_G9991, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->tx_bytes );
    rdd_error = rdd_error ? : rdd_4_bytes_counter_get ( UPSTREAM_VALID_PACKETS_GROUP_G9991, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->rx_packets );
    rdd_error = rdd_error ? : rdd_4_bytes_counter_get ( UPSTREAM_VALID_PACKETS_GROUP_G9991_MC, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->rx_mcast_packets );
    rdd_error = rdd_error ? : rdd_4_bytes_counter_get ( UPSTREAM_VALID_PACKETS_GROUP_G9991_BC, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->rx_bcast_packets );
    rdd_error = rdd_error ? : rdd_4_bytes_counter_get ( UPSTREAM_VALID_BYTES_GROUP_G9991, xi_flow_id, xi_clear_counters, &xi_g9991_flow_counters->rx_bytes );

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_g9991_counters_get ( RDD_G9991_PM_COUNTERS_DTE *xo_g9991_counters)
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_DFC_FRAME_ERROR_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->dfc_frame_error_counter );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_DFC_FRAME_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->dfc_frame_counter );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_DATA_FRAME_ERROR_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->data_frame_error_counter );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_ILLEGAL_SID_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->illegal_sid_error_counter );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_LENGTH_ERROR_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->length_error_counter );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_REASSEMBLY_ERROR_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->reassembly_error_counter );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_BBH_ERROR_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->bbh_error_counter );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    rdd_error = rdd_2_bytes_counter_get ( G9991_GLOBAL_GROUP, US_CONSEQUENT_DROP_G9991_GLOBAL_GROUP_OFFSET, LILAC_RDD_TRUE, &xo_g9991_counters->consequent_drop );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }


    return ( BL_LILAC_RDD_OK );

}
#endif


BL_LILAC_RDD_ERROR_DTE rdd_ring_init ( uint32_t  xi_ring_id,
                                       uint8_t unused0,
                                       bdmf_phys_addr_t xi_ring_address,
                                       uint32_t  xi_number_of_entries,
                                       uint32_t  xi_size_of_entry,
                                       uint32_t  xi_interrupt_id,
                                       uint32_t unused2,
                                       bdmf_phys_addr_t unused3,
                                       uint8_t unused4
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

    f_rdd_lock_irq ( &int_lock_irq, &flags );

#if !defined(FIRMWARE_INIT)
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_RING_DESTROY, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, xi_ring_id, 0, 0, BL_LILAC_RDD_WAIT );
#endif
    f_rdd_unlock_irq ( &int_lock_irq, flags );

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
    RDD_INTERRUPT_COALESCING_TIMER_CONFIG_DTS *ic_timer_config_table_ptr;
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
        ic_timer_config_table_ptr = ( RDD_INTERRUPT_COALESCING_TIMER_CONFIG_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + INTERRUPT_COALESCING_TIMER_CONFIG_TABLE_ADDRESS );
#if defined(FIRMWARE_INIT)
        RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_PERIOD_WRITE( 10, ic_timer_config_table_ptr );
#else
        RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_PERIOD_WRITE( INTERRUPT_COALESCING_TIMER_PERIOD, ic_timer_config_table_ptr );

        RUNNER_REGS_0_CFG_TIMER_TARGET_READ ( runner_timer_target_register );
        runner_timer_target_register.timer_4_6 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_4_6_PICO_CORE_VALUE;
        RUNNER_REGS_0_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );

        /* activate the interrupt coalescing task */
        runner_cpu_wakeup_register.req_trgt = CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER % 32;
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
#endif
        api_first_time_call = LILAC_RDD_FALSE;
    }

    return ( BL_LILAC_RDD_OK );
}

#ifndef G9991

static inline uint32_t f_rdd_spdsvc_kbps_to_tokens(uint32_t xi_kbps)
{
    return ( uint32_t )( ( (1000/8) * xi_kbps ) / SPDSVC_TIMER_HZ );
}

static inline uint32_t f_rdd_spdsvc_mbs_to_bucket_size(uint32_t xi_mbs)
{
    uint32_t bucket_size = xi_mbs;

    if(bucket_size < SPDSVC_BUCKET_SIZE_MIN)
        bucket_size = SPDSVC_BUCKET_SIZE_MIN;

    return bucket_size;
}

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_config( uint32_t xi_kbps,
                                          uint32_t xi_mbs,
                                          uint32_t xi_copies,
                                          uint32_t xi_total_length )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET           runner_timer_target_register;
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;
    static uint32_t                  api_first_time_call = LILAC_RDD_TRUE;
#endif
    RDD_SPEED_SERVICE_PARAMETERS_DTS *spdsvc_parameters_ptr;
    RDD_SPEED_SERVICE_PARAMETERS_DTS spdsvc_parameters;

    spdsvc_parameters_ptr = ( RDD_SPEED_SERVICE_PARAMETERS_DTS * )
        ( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + SPEED_SERVICE_PARAMETERS_TABLE_ADDRESS );

#if !defined(FIRMWARE_INIT)
    if ( api_first_time_call )
    {
        RUNNER_REGS_1_CFG_TIMER_TARGET_READ ( runner_timer_target_register );
        runner_timer_target_register.timer_5_7 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_5_7_MAIN_CORE_VALUE;
        RUNNER_REGS_1_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );

        /* activate the speed service task */
        runner_cpu_wakeup_register.req_trgt = SPEED_SERVICE_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = SPEED_SERVICE_THREAD_NUMBER % 32;
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );

        api_first_time_call = LILAC_RDD_FALSE;
    }
#endif

    RDD_SPEED_SERVICE_PARAMETERS_COPIES_IN_TRANSIT_READ( spdsvc_parameters.copies_in_transit, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TOTAL_COPIES_READ( spdsvc_parameters.total_copies, spdsvc_parameters_ptr );

    if( spdsvc_parameters.copies_in_transit || spdsvc_parameters.total_copies )
    {
        return ( BL_LILAC_RDD_ERROR_SPDSVC_RESOURCE_BUSY );
    }

    RDD_SPEED_SERVICE_PARAMETERS_BBH_DESCRIPTOR_0_WRITE( 0, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_BBH_DESCRIPTOR_1_WRITE( 0, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_SKB_FREE_INDEX_WRITE( 0, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_DISCARDS_WRITE( 0, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_WRITES_WRITE( 0, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_READS_WRITE( 0, spdsvc_parameters_ptr );

    spdsvc_parameters.tokens = f_rdd_spdsvc_kbps_to_tokens( xi_kbps );
    spdsvc_parameters.bucket_size = f_rdd_spdsvc_mbs_to_bucket_size( spdsvc_parameters.tokens + xi_mbs );

    RDD_SPEED_SERVICE_PARAMETERS_TOTAL_COPIES_WRITE( xi_copies, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TOTAL_LENGTH_WRITE( xi_total_length, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TOKENS_WRITE( spdsvc_parameters.tokens, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_BUCKET_SIZE_WRITE( spdsvc_parameters.bucket_size, spdsvc_parameters_ptr );

    return ( BL_LILAC_RDD_OK );
}

void rdd_speed_service_initialize ( void )
{
    uint32_t i = 0;
    uint8_t * stream_prefix_ptr = (uint8_t *)(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET )
        + SPEED_SERVICE_STREAM_PREFIX_ADDRESS - sizeof(RUNNER_COMMON));
    uint8_t stream_prefix[CMD_STREAM_LENGTH] = CMD_STREAM;
    RDD_SPEED_SERVICE_PARAMETERS_DTS *spdsvc_parameters_ptr;

    MEMSET( stream_prefix_ptr, 0, CMD_STREAM_LENGTH );
    for ( i = 0 ; i < CMD_STREAM_LENGTH; i++ )
        MWRITE_I_8( stream_prefix_ptr, i, stream_prefix[i] );

    spdsvc_parameters_ptr = ( RDD_SPEED_SERVICE_PARAMETERS_DTS * )
        ( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + SPEED_SERVICE_PARAMETERS_TABLE_ADDRESS );

    MEMSET( spdsvc_parameters_ptr, 0, sizeof( RDD_SPEED_SERVICE_PARAMETERS_DTS ) );
#ifndef FIRMWARE_INIT
    RDD_SPEED_SERVICE_PARAMETERS_TIMER_PERIOD_WRITE( SPDSVC_TIMER_PERIOD, spdsvc_parameters_ptr );
#else
    RDD_SPEED_SERVICE_PARAMETERS_TIMER_PERIOD_WRITE( 1, spdsvc_parameters_ptr );
#endif

}

BL_LILAC_RDD_ERROR_DTE rdd_spdsvc_get_tx_result( uint8_t *xo_running_p,
                                                 uint32_t *xo_tx_packets_p,
                                                 uint32_t *xo_tx_discards_p )
{
    RDD_SPEED_SERVICE_PARAMETERS_DTS *spdsvc_parameters_ptr;
    RDD_SPEED_SERVICE_PARAMETERS_DTS spdsvc_parameters;

    spdsvc_parameters_ptr = ( RDD_SPEED_SERVICE_PARAMETERS_DTS * )
        ( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + SPEED_SERVICE_PARAMETERS_TABLE_ADDRESS );

    RDD_SPEED_SERVICE_PARAMETERS_TOTAL_COPIES_READ( spdsvc_parameters.total_copies, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_READS_READ( spdsvc_parameters.tx_queue_reads, spdsvc_parameters_ptr );
    RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_DISCARDS_READ( spdsvc_parameters.tx_queue_discards, spdsvc_parameters_ptr );

    *xo_running_p = (spdsvc_parameters.total_copies) ? 1 : 0;
    *xo_tx_packets_p = spdsvc_parameters.tx_queue_reads;
    *xo_tx_discards_p = spdsvc_parameters.tx_queue_discards;

    return ( BL_LILAC_RDD_OK );
}

#endif
