// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
    
*/

#ifndef _BL_LILAC_DRV_RUNNER_TM_H
#define _BL_LILAC_DRV_RUNNER_TM_H



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_eth_tx_queue_config                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configure downstream Ethernet TX queue behind an EMAC. */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures EMAC TX queue:                                                */
/*   - the packet threshold indicates how many packets can be TX pending      */
/*     behind a TX queue.                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_emac_id - EMAC port index (ETH0 - ETH4)                               */
/*   xi_queue_id - ETH TX queue index                                         */
/*   xi_packet_threshold - ETH TX queue packet threshold. Overriden if WRED   */
/*   and/or drop precedence is used                                           */
/*   xi_profile_id - profile ID. If WRED and drop precedence not used, should */
/*   be rdd_queue_profile_disabled. If enabled, xi_packet_threshold MUST be   */
/*   equal to minimum low threshold in profile.                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_EMAC_ID - EMAC index is illegal.            */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID - queue index is greater then 7    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_eth_tx_queue_config ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                 BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                                 uint16_t                   xi_packet_threshold,
                                                 rdd_queue_profile          xi_profile_id,
                                                 uint8_t                    xi_counter_id );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function returns emac egress counter value for mdu mode.            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_emac_id                                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_mdu_mode_pointer                                                      */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mdu_mode_pointer_get ( BL_LILAC_RDD_EMAC_ID_DTE  xi_emac_id,
                                                  uint16_t                  *xo_mdu_mode_pointer );

#endif /* _BL_LILAC_DRV_RUNNER_TM_H */

