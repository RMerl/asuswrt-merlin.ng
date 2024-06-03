// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
   
*/

#ifndef _BL_LILAC_DRV_RUNNER_COMMON_H
#define _BL_LILAC_DRV_RUNNER_COMMON_H

#include "rdd_data_structures.h"
#if !defined(FIRMWARE_INIT)
#include "rdp_drv_bpm.h"
#endif /* !defined(FIRMWARE_INIT) */

typedef struct {
    uint32_t write;
    uint32_t read;
    uint32_t count;
    uint16_t *data;
} cpu_tx_skb_free_indexes_cache_t;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_smart_card_global_params_config                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*  Smart card                                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the global timing parameters that should apply on     */
/*  transactions from now on                        		              */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*   xi_waiting_time - The maximum delay between the leading edge of a        */
/* 	 	character transmitted by the card and the leading edge of the */
/* 	    previous character  					      */
/* 	 xi_guard_time	-  The minimum delay between the leading edges of     */
/* 		 two consecutive characters                                   */
/*   xi_etu	-  elementary time unit , in which one bit is transffered     */
/*                                                                            */
/* Output:                                                                    */
/*     BL_LILAC_RDD_ERROR_DTE - Return status                                 */
/*     		BL_LILAC_RDD_OK - No error                                    */
/*                                                                            */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_smart_card_global_params_config ( uint32_t  xi_waiting_time,
                                                             uint8_t   xi_guard_time,
                                                             uint8_t   xi_etu,
                                                             uint8_t   xi_max_retransmit );


/************************************************************************************/
/*                                                                                  */
/* Name:  rdd_smart_card_command_params_config                                      */                           
/*                                                                       	    */
/* Title:                                                                           */
/*                                                                                  */
/*  Smart card                                                                      */
/*                                                                                  */
/* Abstract:                                                                        */
/* 		This function sets the parameters of the wanted transaction         */
/*		and starts it.							    */
/* Input:  									    */
/*		*xi_header_arr  - array of 5 bytes containing the header            */
/*		*xi_data_arr	- data array to send                                */
/*		xi_send_len   	- number of bytes to be send to the smart card.     */
/*		  		  Only for PPS and NORMAL task types .		    */
/*		xi_receive_len   - number of bytes to be recieved to/from the       */
/*			           smart card. Ignored for not NORMAL task type     */
/*		xi_task_type -  TX_TASK / RX_TASK / PPS / ANSWER_TO_RESET / NORMAL  */
/*										    */
/*		TX_TASK:                                                            */
/*			Sending header and data to the card .                       */
/*                                                                                  */
/*		RX_TASK:                                                            */
/*			Sending header and receiving data from the card.            */
/*                                                                                  */
/*      PPS :                                                                       */
/*			The pps message is contained in xi_data array.              */
/*			xi_send_len   is the length of pps message.                 */
/*			xi_header      will be ignored.                             */
/*                                                                                  */
/*		RESET:                                                              */
/* 			receive and parse (calculating the size) of ANSWER_TO_RESET */
/* 			PDU initiated after cold/warm reset  of smart card.         */
/*		 	Note: xi_data_len, xi_header, xi_data are ignored           */
/*	                                                                            */
/*      NORMAL : sending xi_send_len bytes  and receiving xi_receive_len  bytes .   */
/*		        (used mainly for debugging)		                    */
/* 		Output:                                                             */
/*     		BL_LILAC_RDD_OK                                                     */
/*                                                                                  */
/************************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_smart_card_command_params_config ( uint8_t   xi_task_type,
                                                              uint16_t  xi_send_length,
                                                              uint16_t  xi_receive_length,
                                                              uint8_t   *xi_header_array,
                                                              uint8_t   *xi_data_array );


/************************************************************************************/
/*                                                                                  */
/* Name: 	 rdd_smart_card_command_params_read                                 */
/*                                                                                  */
/* Title:    Smart card                                                             */
/*                                                                            	    */
/* Abstract:                                                                        */
/*		This function reads  the bytes received from smart card.            */
/*       Use after transaction is completed.                                        */
/*                   					                            */
/*	Input:       	                                                            */
/*		*xi_send_len   - ptr to number of bytes that was sent               */
/*		*xi_recieve_len  -  ptr to number of bytes that was received	    */
/*              *xi_data_arr  - data array  of received bytes with length           */
/*                xi_recieve_len                                                    */
/*      *status_byte  - status of the transaction when it was ended (success/error) */
/*		                                                                    */
/*	Output:                                                                     */
/*    	BL_LILAC_RDD_OK                                                             */
/*                                                                                  */
/************************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_smart_card_command_params_read ( uint16_t  *xo_send_length,
                                                            uint16_t  *xo_receive_length,
                                                            uint8_t   *xo_data_array,
                                                            uint8_t   *status_byte,
                                                            uint32_t  *xo_send_error_counter,
                                                            uint32_t  *xo_recv_error_counter );


/************************************************************************************/
/*                                                                                  */
/*  Name: 	 rdd_smart_card_status_get                                          */
/*                                                                                  */
/* Title:    Smart card                                                             */
/*                                                                                  */
/* Abstract:                                                                        */
/*		This function reads  status from smart card.                        */
/*                   					                            */
/*	Input:       	                                                            */
/*		none                                                                */
/*		                                                                    */
/*	Output:                                                                     */
/*    	BL_LILAC_RDD_OK                                                             */
/*                                                                                  */
/************************************************************************************/
BL_LILAC_RDD_ERROR_DTE  rdd_smart_card_status_get ( uint8_t  *xo_smart_card_status );


/************************************************************************************/
/*                                                                                  */
/* Name:  rdd_smart_card_task_start                                                 */                 
/*                                                                                  */
/* Title:    Smart card                                                             */
/*                                                                            	    */
/* Abstract:                                                                        */
/* 		This function send wakeup to smart card task                        */
/*		and starts it.						            */
/*                                                                                  */
/* Input:                                                                           */
/*		none		        				     	    */
/*                                                                                  */
/* 	Output:                                                                     */
/*     		BL_LILAC_RDD_OK                                                     */
/*                                                                                  */
/************************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_smart_card_task_start ( void );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_critical_section_config                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - RDD lock mechanism                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* This function initializes pointers to critical section callback functions  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*     xi_lock_function - pointer to critical section begin function          */
/*     xi_unlock_function - pointer to critical section exit function         */
/*     xi_lock_irq_function - pointer to critical section within IRQ begin    */
/*                            function                                        */
/*     xi_unlock_irq_function - pointer to critical section exit within IRQ   */
/*                            function                                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_critical_section_config ( BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_DTE        xi_lock_function,
                                                     BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_DTE      xi_unlock_function,
                                                     BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_IRQ_DTE    xi_lock_irq_function,
                                                     BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_IRQ_DTE  xi_unlock_irq_function );



/* INTERNAL COMMON FUNCTIONS */

static inline BL_LILAC_RDD_BRIDGE_PORT_DTE rdd_bridge_port_vector_to_bridge_port ( BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  xi_bridge_port_vector )
{
    BL_LILAC_RDD_BRIDGE_PORT_DTE  bridge_port;

    switch ( xi_bridge_port_vector ) 
    {
    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI:
        bridge_port = BL_LILAC_RDD_PCI_BRIDGE_PORT;
        break;

    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN0:
        bridge_port = BL_LILAC_RDD_LAN0_BRIDGE_PORT;
        break;

    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN1:
        bridge_port = BL_LILAC_RDD_LAN1_BRIDGE_PORT;
        break;

    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN2:
        bridge_port = BL_LILAC_RDD_LAN2_BRIDGE_PORT;
        break;

    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN3:
        bridge_port = BL_LILAC_RDD_LAN3_BRIDGE_PORT;
        break;

    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4:
        bridge_port = BL_LILAC_RDD_LAN4_BRIDGE_PORT;
        break;

    default:
        bridge_port = 0;
        break;
    }

    return ( bridge_port );
}


static inline int32_t rdd_bridge_port_to_port_index ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                      BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id )
{
    /* upstream */
    if ( ( xi_bridge_port >= BL_LILAC_RDD_LAN0_BRIDGE_PORT ) && ( xi_bridge_port <= BL_LILAC_RDD_LAN4_BRIDGE_PORT ) )
    {
        if ( xi_subnet_id == 0 )
        {
            return ( xi_bridge_port );
        }
    }
    /*also apply for G999.1 General configuration:*/
    else if ( xi_bridge_port == BL_LILAC_RDD_PCI_BRIDGE_PORT )
    {
        return ( 0 );
    }

    /* downstream */
    else if ( BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(xi_bridge_port) ) // DSL
    {
        if ( xi_subnet_id == 0 )
        {
            return ( BL_LILAC_RDD_SUBNET_BRIDGE );
        }
    }
    else if ( xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT )
    {
        if ( xi_subnet_id == 0 )
        {
            return ( BL_LILAC_RDD_SUBNET_BRIDGE_IPTV );
        }
    }
    else if ( xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        if ( xi_subnet_id == 0 )
        {
            return ( BL_LILAC_RDD_SUBNET_FLOW_CACHE );
        }
    }

    return ( -1 );
}


static inline int32_t rdd_bridge_port_to_class_id ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port )
{
    switch ( xi_bridge_port )
    {
    case BL_LILAC_RDD_WAN0_BRIDGE_PORT: // DSL
    case BL_LILAC_RDD_WAN1_BRIDGE_PORT: // DSL

        return ( LILAC_RDD_IH_WAN_BRIDGE_LOW_CLASS );

    case BL_LILAC_RDD_LAN0_BRIDGE_PORT:

        return ( LILAC_RDD_IH_LAN_EMAC0_CLASS );

    case BL_LILAC_RDD_LAN1_BRIDGE_PORT:

        return ( LILAC_RDD_IH_LAN_EMAC1_CLASS );

    case BL_LILAC_RDD_LAN2_BRIDGE_PORT:

        return ( LILAC_RDD_IH_LAN_EMAC2_CLASS );

    case BL_LILAC_RDD_LAN3_BRIDGE_PORT:

        return ( LILAC_RDD_IH_LAN_EMAC3_CLASS );

    case BL_LILAC_RDD_LAN4_BRIDGE_PORT:

        return ( LILAC_RDD_IH_LAN_EMAC4_CLASS );

    case BL_LILAC_RDD_PCI_BRIDGE_PORT:

        return ( LILAC_RDD_IH_PCI_CLASS );

    default:

        return ( 0 );
    }

    return ( 0 );
}


#if !defined(FIRMWARE_INIT)
static inline int32_t rdd_bridge_port_to_bpm_src_port ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port )
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
#endif


BL_LILAC_RDD_ERROR_DTE rdd_timer_task_config ( rdpa_traffic_dir  xi_direction,
                                               uint16_t          xi_task_period_in_usec,
                                               uint16_t          xi_firmware_routine_address_id );


static inline uint32_t rdd_budget_to_alloc_unit ( uint32_t budget, uint32_t period, uint32_t exponent )
{
    return ( ( ( budget + ( ( 1000000 / period ) / 2 ) ) / ( 1000000 / period ) ) >> exponent );
}


static inline uint32_t rdd_get_exponent ( uint32_t  value,
                                          uint32_t  mantissa_len,
                                          uint32_t  exponent_list_len,
                                          uint32_t  *exponent_list )
{
    uint32_t  i;

    for ( i = exponent_list_len - 1; i > 0; i-- )
    {
        if ( value > ( ( ( 1 << mantissa_len ) - 1 ) << exponent_list[ i - 1 ] ) )
        {
            return ( i );
        }
    }

    return ( 0 );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_TYPE  xi_msg_type,
                                                   LILAC_RDD_RUNNER_INDEX_DTS     xi_runner_index,
                                                   uint32_t                       xi_sram_base,
                                                   uint32_t                       xi_parameter_1,
                                                   uint32_t                       xi_parameter_2,
                                                   uint32_t                       xi_parameter_3,
                                                   BL_LILAC_RDD_CPU_WAIT_DTE      xi_wait );

#define rdd_cpu_tx_send_message f_rdd_cpu_tx_send_message


#endif /* _BL_LILAC_DRV_RUNNER_COMMON_H */

