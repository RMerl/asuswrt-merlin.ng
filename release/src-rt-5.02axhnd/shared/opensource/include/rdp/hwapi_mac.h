/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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


/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This header file defines all datatypes and functions exported for the      */
/* Ethernet MAC		                                                          */
/*                                                                            */
/******************************************************************************/


#ifndef __UNIFIED_DRV_MAC_H
#define __UNIFIED_DRV_MAC_H


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "rdpa_types.h"
/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

typedef struct
{
	int32_t rxFlowEnable;
	int32_t txFlowEnable;
}S_MAC_HWAPI_FLOW_CTRL;

typedef enum
{
	MAC_LPBK_NONE,
	MAC_LPBK_LOCAL,
	MAC_LPBK_REMOTE,
	MAC_LPBK_BOTH
}MAC_LPBK;

#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
typedef struct
{
	uint32_t    reserved:26;// Reserved bits must be written with 0.  A read returns an unknown value.
	uint32_t    mac_link_stat:1;// Link status indication.Reset value is 0x0.
	uint32_t    mac_tx_pause:1;// 1: MAC Tx pause enabled. 0: MAC Tx pause disabled. Reset value is 0x1.
	uint32_t	mac_rx_pause:1;// 1: MAC Rx pause enabled. 0: MAC Rx pause disabled.	Reset value is 0x1.
	uint32_t	mac_duplex:1;//1: Half duplex. 0: Full duplex. Reset value is 0x0.
	uint32_t	mac_speed:2;// 00: 10Mbps, 01: 100Mbps, 10: 1Gbps, 11: 2.5Gbps	Reset value is 0x2.

}S_HWAPI_MAC_STATUS;
#else
typedef struct
{
	uint32_t	mac_speed:2;// 00: 10Mbps, 01: 100Mbps, 10: 1Gbps, 11: 2.5Gbps	Reset value is 0x2.
	uint32_t	mac_duplex:1;//1: Half duplex. 0: Full duplex. Reset value is 0x0.
	uint32_t	mac_rx_pause:1;// 1: MAC Rx pause enabled. 0: MAC Rx pause disabled.	Reset value is 0x1.
	uint32_t    mac_tx_pause:1;// 1: MAC Tx pause enabled. 0: MAC Tx pause disabled. Reset value is 0x1.
	uint32_t    mac_link_stat:1;// Link status indication.Reset value is 0x0.
	uint32_t    reserved:26;// Reserved bits must be written with 0.  A read returns an unknown value.

}S_HWAPI_MAC_STATUS;
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_configuration                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get configuration                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   get the configuratin of a mac port	,note that current status of emac	  */
/*   might be different than configuration when working in autoneg			  */
/*	to get the current status use mac_hwapi_get_mac_status API				  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    emac_cfg -  structure holds the current configuration of the mac  port  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_configuration(rdpa_emac emacNum,rdpa_emac_cfg_t *emac_cfg);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_configuration                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set configuration                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   set the configuratin of a mac port                                       */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   emac_cfg -  structure holds the current configuration of the mac  port   */
/*                                                                            */
/* Output:                                                                    */
/*      																	  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_configuration(rdpa_emac emacNum,rdpa_emac_cfg_t *emac_cfg);
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_duplex		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port duplex                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the dulplex of a port												  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    full_duples  :  1 = full dulplex,0 = half_duplex						  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_duplex(rdpa_emac emacNum,int32_t *full_duplex);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_duplex		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port duplex                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the dulplex of a port												  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    full_duples  :  1 = full dulplex,0 = half_duplex						  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_duplex(rdpa_emac emacNum,int32_t full_duplex);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_speed		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port speed rate                                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the speed of a port													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                              							                  */
/*                                                                            */
/* Output:                                                                    */
/*      rate -   enum of the speed                                            */
/******************************************************************************/
void mac_hwapi_get_speed(rdpa_emac emacNum,rdpa_emac_rate *rate);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_speed		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port speed rate                                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the speed of a port													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*      rate -   enum of the speed   						                  */
/*                                                                            */
/* Output:                                                                    */
/*                                   							              */
/******************************************************************************/

void mac_hwapi_set_speed(rdpa_emac emacNum,rdpa_emac_rate rate);



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_external_conf                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - en/disable external speed configuration                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set port speed external configuration				      				  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*    enable - boolean enable                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                   							              */
/******************************************************************************/

void mac_hwapi_set_external_conf(rdpa_emac emacNum, int enable);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rxtx_enable                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx and rx enable                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get tx and rx enable													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                             								  */
/*                                                                            */
/* Output:                                                                    */
/*       rxtxEnable  - boolean enable   	                                  */
/******************************************************************************/
void mac_hwapi_get_rxtx_enable(rdpa_emac emacNum,int32_t *rxEnable,int32_t *txEnable);





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_rxtx_enable                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx and rx enable                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set tx and rx enable													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*    rxtxEnable  - boolean enable                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_rxtx_enable(rdpa_emac emacNum,int32_t rxEnable,int32_t txEnable);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_sw_reset		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port software reset                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the sw reset bit of emac port										  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    swReset  :  1 = reset,0 = not reset									  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_sw_reset(rdpa_emac emacNum,int32_t *swReset);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_sw_reset		                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port software reset                                  	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the sw reset bit of emac port										  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   swReset  :  1 = reset,0 = not reset	                                  */
/*                                                                            */
/* Output:                                                                    */
/*    								  										  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_sw_reset(rdpa_emac emacNum,int32_t swReset);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_min_pkt_size                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get tx minimum packet size                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  Get the unimac configuration for minimum tx packet size                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    min_pkt_size  :  14...125                                               */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_min_pkt_size(rdpa_emac emacNum,int32_t *min_pkt_size);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_min_pkt_size                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set tx minimum packet size                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*  Set the unimac configuration for minimum tx packet size                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   min_pkt_size  :  14...125                                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_tx_min_pkt_size(rdpa_emac emacNum,int32_t min_pkt_size);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_max_frame_len		                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port TX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the port maximum transmit unit size in bytes						  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                              										      */
/*                                                                            */
/* Output:                                                                    */
/*    maxTxFrameLen - size of frame in bytes 								  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_max_frame_len(rdpa_emac emacNum,uint32_t *maxTxFrameLen );




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_max_frame_len		                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port TX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the port maximum transmit unit size in bytes						  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*    maxTxFrameLen - size of frame in bytes                         	      */
/*                                                                            */
/* Output:                                                                    */
/*   																		  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_tx_max_frame_len(rdpa_emac emacNum,uint32_t maxTxFrameLen );




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rx_max_frame_len		                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port RX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the port maximum receive unit size in bytes							  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                           	     										  */
/*                                                                            */
/* Output:                                                                    */
/*   maxRxFrameLen - size of current MRU									  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_rx_max_frame_len(rdpa_emac emacNum,uint32_t *maxRxFrameLen );




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_rx_max_frame_len		                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port RX MTU                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the port maximum receive unit size in bytes							  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*    maxRxFrameLen - size of current MRU        							  */
/*                                                                            */
/* Output:                                                                    */
/*   									  									  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_rx_max_frame_len(rdpa_emac emacNum,uint32_t maxRxFrameLen );




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_tx_igp_len	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port inter frame gap                                 	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the inter frame gap	size in bytes									  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   txIpgLen - length in bytes                                               */
/*                                                                            */
/* Output:                                                                    */
/*    																		  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_tx_igp_len(rdpa_emac emacNum,uint32_t txIpgLen );



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_igp_len	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port inter frame gap                                 	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the inter frame gap	size in bytes									  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                      							          */
/*                                                                            */
/* Output:                                                                    */
/*    txIpgLen - length in bytes    										  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_igp_len(rdpa_emac emacNum,uint32_t *txIpgLen );




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_mac_status	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port status                                         	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the status of mac													  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    macStatus  :  														  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_mac_status(rdpa_emac emacNum,S_HWAPI_MAC_STATUS *macStatus);





/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_flow_control                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - get port flow control                                    	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the flow control of a port											  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*           																  */
/*                                                                            */
/* Output:                                                                    */
/*   flowControl - structure with parameters of tx and rx flow control		  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_flow_control                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port flow control                                    	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the flow control of a port											  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   flowControl - structure with parameters of tx and rx flow control        */
/*                                                                            */
/* Output:                                                                    */
/*   						  												  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_flow_control(rdpa_emac emacNum,S_MAC_HWAPI_FLOW_CTRL *flowControl);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_pause_params                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set port flow control                                    	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set the flow control of a port											  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*   flowControl - structure with parameters of tx and rx flow control        */
/*                                                                            */
/* Output:                                                                    */
/*   						  												  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_pause_params(rdpa_emac emacNum,int32_t pauseCtrlEnable,uint32_t pauseTimer,uint32_t pauseQuanta);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_rx_counters	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	get the rx counters of port                           	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the rx counters of port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    rxCounters  :  structure filled with counters							  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_rx_counters(rdpa_emac emacNum,rdpa_emac_rx_stat_t *rxCounters);




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_tx_counters	                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	get the tx counters of port                           	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	get the tx counters of port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*    txCounters  :  structure filled with counters							  */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_tx_counters(rdpa_emac emacNum,rdpa_emac_tx_stat_t *txCounters);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_init_emac	                         	                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	init the emac to well known state                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	initialized the emac port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_init_emac(rdpa_emac emacNum);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_loopback	                         	                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	init the emac to well known state                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	initialized the emac port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_get_loopback(rdpa_emac emacNum,MAC_LPBK *loopback);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_get_loopback	                         	                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - 	init the emac to well known state                      	  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	initialized the emac port				  								  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index           	                                  */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_loopback(rdpa_emac emacNum,MAC_LPBK loopback);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_modify_flow_control_pause_pkt_addr                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - Modify the Flow Control pause pkt source address            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function modifies the flow control pause pkt source address         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - EMAC id (0-5)                                                 */
/*                                                                            */
/*   mac - Flow Control mac address                                   */
/*                                                                            */
/* Output:   N/A                                                              */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_modify_flow_control_pause_pkt_addr ( rdpa_emac emacNum,
    bdmf_mac_t mac);
void mac_hwapi_set_unimac_cfg(rdpa_emac emacNum, int32_t enabled);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_backpressure_ext                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set/reset backpressure to external switch                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set/reset backpressure to external switch                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   enable  - boolean enable                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_backpressure_ext(rdpa_emac emacNum, int32_t enable);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mac_hwapi_set_eee                                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MAC Driver - set eee configuration                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* 	set eee configuration                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   emacNum - emac Port index                                                */
/*   enable  - boolean enable                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
void mac_hwapi_set_eee(rdpa_emac emacNum, int32_t enable);

#endif
