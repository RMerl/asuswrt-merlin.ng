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

extern uint8_t *ContextTableBase;
extern uint8_t *g_runner_ddr_base_addr;
extern uint8_t *g_runner_tables_ptr;
extern uint32_t                                     g_ddr_headroom_size;
extern uint32_t                                     g_rate_controllers_pool_idx;
extern BL_LILAC_RDD_BRIDGE_PORT_DTE                 g_broadcom_switch_physical_port;
extern RDD_WAN_TX_POINTERS_TABLE_DTS                *wan_tx_pointers_table_ptr;
extern bdmf_fastlock                                int_lock_irq;
extern rdpa_bpm_buffer_size_t g_bpm_buffer_size;


BL_LILAC_RDD_ERROR_DTE f_rdd_ds_exponent_table_initialize ( void )
{
    RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS    *exponent_table_ptr;
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_DTS    *exponent_entry_ptr;

    /* initialize exponents table */
    exponent_table_ptr = ( RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 0 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT0, exponent_entry_ptr );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 1 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT1, exponent_entry_ptr );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 2 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT2, exponent_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_eth_tx_queue_config ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                 BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                                 uint16_t                   xi_packet_threshold,
                                                 rdd_queue_profile          xi_profile_id,
                                                 uint8_t                    xi_counter_id )
{
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS       *eth_tx_queue_descriptor_ptr;
    RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS  *eth_tx_queues_pointers_table_ptr;
    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS   *eth_tx_queue_pointers_entry_ptr;
    uint16_t                              eth_tx_queue_descriptor_offset;
    uint16_t                              queue_profile_address;

    /* check the validity of the input parameters - emac id */
    if ( ( xi_emac_id < BL_LILAC_RDD_EMAC_ID_START ) || ( xi_emac_id >= BL_LILAC_RDD_EMAC_ID_COUNT ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_EMAC_ID );
    }

    /* check the validity of the input parameters - emac tx queue id */
    if ( xi_queue_id > BL_LILAC_RDD_QUEUE_LAST )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID );
    }

	if ( xi_emac_id == BL_LILAC_RDD_EMAC_ID_PCI )
    {
        /* PCI TX has 4 queues */
        if (  xi_queue_id > BL_LILAC_RDD_QUEUE_3 )
        {
            return ( BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID );
        }

        if ( ( xi_profile_id == rdd_queue_profile_disabled ) && ( xi_packet_threshold > 0 ) )
        {
            /* PCI TX fifo imposes queue configuration larger than fifo size */
            if ( xi_packet_threshold <= LILAC_RDD_PCI_TX_FIFO_SIZE )
            {
                return ( BL_LILAC_RDD_ERROR_PCI_QUEUE_THRESHOLD_TOO_SMALL );
            }

            xi_packet_threshold -= LILAC_RDD_PCI_TX_FIFO_SIZE;
        }
    }

    eth_tx_queues_pointers_table_ptr = ( RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS );

    eth_tx_queue_pointers_entry_ptr = &( eth_tx_queues_pointers_table_ptr->entry[ xi_emac_id * RDD_EMAC_NUMBER_OF_QUEUES + xi_queue_id ] );

    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_READ ( eth_tx_queue_descriptor_offset, eth_tx_queue_pointers_entry_ptr );

    eth_tx_queue_descriptor_ptr = ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + eth_tx_queue_descriptor_offset );

    if ( xi_profile_id == rdd_queue_profile_disabled )
    {
        queue_profile_address = 0;
    }
    else
    {
        queue_profile_address = DS_QUEUE_PROFILE_TABLE_ADDRESS + xi_profile_id * sizeof ( RDD_QUEUE_PROFILE_DTS );
    }

    RDD_ETH_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE ( xi_packet_threshold, eth_tx_queue_descriptor_ptr );
    RDD_ETH_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE ( queue_profile_address, eth_tx_queue_descriptor_ptr );

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_mdu_mode_pointer_get ( BL_LILAC_RDD_EMAC_ID_DTE  xi_emac_id,
                                                  uint16_t                  *xo_mdu_mode_pointer )
{
    /* check the validity of the input parameters - emac id */
    if ( ( xi_emac_id < BL_LILAC_RDD_EMAC_ID_0 ) || ( xi_emac_id >= BL_LILAC_RDD_EMAC_ID_COUNT ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_EMAC_ID );
    }

    *xo_mdu_mode_pointer = ETH_TX_MAC_TABLE_ADDRESS + xi_emac_id * sizeof ( RDD_ETH_TX_MAC_DESCRIPTOR_DTS ) + LILAC_RDD_EMAC_EGRESS_COUNTER_OFFSET;

    return ( BL_LILAC_RDD_OK );
}
