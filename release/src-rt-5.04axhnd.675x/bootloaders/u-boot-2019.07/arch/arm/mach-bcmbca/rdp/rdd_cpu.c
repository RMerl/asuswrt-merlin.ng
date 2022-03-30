// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
    
*/

#include "rdd.h"

#define _RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE  RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE2

/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

#if !defined(FIRMWARE_INIT)
static uint8_t                                 g_dummy_read;
#endif
extern uint8_t                                 g_broadcom_switch_mode;
extern BL_LILAC_RDD_BRIDGE_PORT_DTE            g_broadcom_switch_physical_port;

extern RDD_WAN_TX_POINTERS_TABLE_DTS           *wan_tx_pointers_table_ptr;
extern rdpa_bpm_buffer_size_t g_bpm_buffer_size;

BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_initialize ( void )
{
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS               *cpu_reason_to_cpu_rx_queue_table_ptr;
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS               *cpu_reason_to_cpu_rx_queue_entry_ptr;
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS                      *cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS                      *cpu_reason_to_meter_entry_ptr;
    uint16_t                                               *cpu_rx_ingress_queue_ptr;
    uint8_t                                                cpu_reason;
    uint8_t                                                cpu_queue;

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

BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_initialize ( void )
{
    uint32_t *ih_header_descriptor_ptr;
    uint32_t ih_header_descriptor[2];
    uint32_t *ih_buffer_bbh_ptr;
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

BL_LILAC_RDD_ERROR_DTE rdd_cpu_reason_to_cpu_rx_queue ( rdpa_cpu_reason  xi_cpu_reason,
                                                        BL_LILAC_RDD_CPU_RX_QUEUE_DTE   xi_queue_id,
                                                        rdpa_traffic_dir                xi_direction,
                                                        uint32_t                        xi_table_index )
{
    return _rdd_cpu_reason_to_cpu_rx_queue(xi_cpu_reason, xi_queue_id, xi_direction, xi_table_index);
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
