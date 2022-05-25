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

#ifndef _BL_LILAC_DRV_RUNNER_TM_H
#define _BL_LILAC_DRV_RUNNER_TM_H


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_channel_set                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configure upstream TCONT                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the schedule mechansim of a single TCONT.       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_schedule - Strict priority or Rate control                            */
/*   xi_rate_limiter_mode - disable/enable US overall rate limiter            */
/*   xi_rate_limiter_priority - high priority (never dropped) or low priority */
/*   (dropped when no budget) for US overall rate limiter                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_channel_set ( RDD_WAN_CHANNEL_ID           xi_wan_channel_id,
                                             RDD_WAN_CHANNEL_SCHEDULE     xi_schedule,
                                             RDD_US_PEAK_SCHEDULING_MODE  xi_peak_scheduling_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_tcont_byte_counter_read                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - read upstream TCONT statistic                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function read (take a snapshot) of how many TX bytes are pending    */
/*   behind a TCONT.                                                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xo_byte_counter - the number of TX pending bytes behind the TCONT.       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_channel_byte_counter_read ( RDD_WAN_CHANNEL_ID  xi_wan_channel_id,
                                                           uint32_t            *xo_byte_counter );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_channel_rate_limiter_config                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configures upstream T-CONT rate limiting               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures TCONT for upstream rate limiting                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT number to be configured                             */
/*   xi_rate_limiter_mode - disable/enable                                    */
/*   xi_rate_limiter_priority - high priority (never dropped) or low priority */
/*   (dropped when no budget)                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_channel_rate_limiter_config ( RDD_WAN_CHANNEL_ID                      xi_wan_channel_id,
                                                             BL_LILAC_RDD_RATE_LIMITER_MODE_DTE      xi_rate_limiter_mode,
                                                             BL_LILAC_RDD_RATE_LIMITER_PRIORITY_DTE  xi_rate_limiter_priority );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_rate_controller_config                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configure upstream rate controller behind a TCONT      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures rate controller:                                              */
/*   - Rate controller 0 should be always configured for strict priority      */
/*     TCONTs.                                                                */
/*   - only 128 rate controllers can be configured by dynamic allocation      */
/*   - all tx queue pointers are initilaized to dummy queue with threshold 0  */
/*   - sustain and peak budgets are only for rate control TCONTS, sustain     */
/*     rate is ensured, while peak is given only when sustain was done, with  */
/*     weight considaration.                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_rate_controller_id - rate controller index                            */
/*   xi_sustain_budget - sustain budget in bytes per second                   */
/*   xi_peak_budget - peak budget in bytes per second                         */
/*   xi_peak_weight - peak weight (move to next rate controller) in bytes     */
/*   xi_peak limit - upper limit for current peak budget in bytes in each     */
/*   allocation round                                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_RATE_CONTROLLER_ID - rate controller index  */
/*                                                     is greater then 31     */
/*     BL_LILAC_RDD_ERROR_RATE_CONTROLLERS_POOL_OVERFLOW - more then 128 rate */
/*                                         controllers are alraedy configured */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_rate_controller_config ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                    BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                    RDD_RATE_CONTROLLER_PARAMS           *xi_params );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_rate_controller_modify                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configure upstream rate controller behind a TCONT      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   modifies rate controller:                                                */
/*   - all the parameters of a rate controller can be modified in run time    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_rate_controller_id - rate controller index                            */
/*   xi_sustain_budget - sustain budget in bytes per second                   */
/*   xi_peak_budget - peak budget in bytes per second                         */
/*   xi_peak_weight - peak weight (move to next rate controller) in bytes     */
/*   xi_peak limit - upper limit for current peak budget in bytes in each     */
/*   allocation round                                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_RATE_CONTROLLER_ID - rate controller index  */
/*                                                     is greater then 31     */
/*     BL_LILAC_RDD_ERROR_RATE_CONTROLLER_NOT_CONFIGURED - the rate           */
/*                    controller cannot be modified since it wasnt configured */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_rate_controller_modify ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                    BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                    RDD_RATE_CONTROLLER_PARAMS           *xi_params );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_rate_controller_remove                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - removes upstream rate controller from a TCONT          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   remove rate controller from TCONT:                                       */
/*   - replace it pointer to the dummy rate controller whithin the TCONT      */
/*   - free the rate controller descriptor back to the pool                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_rate_controller_id - rate controller index                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_RATE_CONTROLLER_ID - rate controller index  */
/*                                                     is greater then 31     */
/*     BL_LILAC_RDD_ERROR_RATE_CONTROLLER_NOT_CONFIGURED - the rate           */
/*                    controller cannot be modified since it wasnt configured */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_rate_controller_remove ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                    BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_queue_config                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configure upstream GPON TX queue behind a TCONT        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures GPON TX queue:                                                */
/*   - only 256 TX queues can be configured by dynamic allocation             */
/*   - the packet threshold indicates how many packets can be TX pending      */
/*     behind a TX queue.                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_rate_controller_id - rate controller index                            */
/*   xi_queue_id - GPON TX queue index                                        */
/*   xi_packet_threshold - GPON TX queue packet threshold                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_RATE_CONTROLLER_ID - rate controller index  */
/*                                                     is greater then 31     */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID - queue index is greater then 7    */
/*     BL_LILAC_RDD_ERROR_GPON_TX_QUEUES_POOL_OVERFLOW - more then 256 TX     */
/*                                          queues are alraedy configured     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_config ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                 BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                 BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                 uint16_t                             xi_packet_threshold,
                                                 rdd_queue_profile                    xi_profile_id,
                                                 uint8_t                              xi_counter_id );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_queue_modify                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configure GPON TX queue behind a TCONT                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   modifies rate controller:                                                */
/*   - all the parameters of a GPON TX queue can be modified in run time      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_rate_controller_id - rate controller index                            */
/*   xi_queue_id - GPON TX queue index                                        */
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
/*     BL_LILAC_RDD_ERROR_GPON_TX_QUEUE_NOT_CONFIGURED - the tx queue cannot  */
/*                                     be modified since it wasnt configured  */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_modify ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                 BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                 BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                 uint16_t                             xi_packet_threshold,
                                                 rdd_queue_profile                    xi_profile_id,
                                                 uint8_t                              xi_counter_id );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_queue_remove                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - removes upstream GPON TX queue from a TCONT            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   remove rate controller from TCONT:                                       */
/*   - replace it pointer to the dummy TX queue whithin the rate controller   */
/*   - free the TX queue descriptor back to the pool                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_rate_controller_id - rate controller index                            */
/*   xi_queue_id - GPON TX queue index                                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_RATE_CONTROLLER_ID - rate controller index  */
/*                                                     is greater then 31     */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID - queue index is greater then 7    */
/*     BL_LILAC_RDD_ERROR_GPON_TX_QUEUE_NOT_CONFIGURED - the tx queue cannot  */
/*                                     be modified since it wasnt configured  */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_remove ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                 BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                 BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_queue_flush                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - flush all the pending tx packets behind a TCONT.       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   flush all tx pending packets from a GPON TX queue, due to a posible      */
/*   race condition, the task is performd by the Runner through a CPU message */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT index                                               */
/*   xi_rate_controller_id - rate controller index                            */
/*   xi_queue_id - GPON TX queue index                                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID - WAN channel id is greater  */
/*                                                 then 39.                   */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_RATE_CONTROLLER_ID - rate controller index  */
/*                                                     is greater then 31     */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID - queue index is greater then 7    */
/*     BL_LILAC_RDD_ERROR_GPON_TX_QUEUE_NOT_CONFIGURED - the tx queue cannot  */
/*                                     be modified since it wasnt configured  */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_flush ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                BL_LILAC_RDD_CPU_WAIT_DTE            xi_wait );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_queue_get_status                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_get_status ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                     BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                     BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                     rdpa_stat_1way_t                     *stat );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_flow_control_config                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_flow_control_config ( BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE xi_bridge_port);
#endif

/******************************************************************************/
/*   f_epon_tx_post_scheduling_ddr_queue_initialize                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   gpon sniffer                                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures  post scheduling ddr queue for EPON  - LLID     */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_queue_size                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE f_epon_tx_post_scheduling_ddr_queue_initialize ( uint16_t  xi_queue_size );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_emac_config                                                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configures rate limiter behind an EMAC.                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the schedule mechansim of a single EMAC.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_emac_id - EMAC port index (ETH0 - ETH4)                               */
/*   xi_rate_limiter_id - rate limiter index (0-5) or 6 for idle.             */
/*   xi_rate_shaper_max_burst - single rate shaper burst before scheduling    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_EMAC_ID - EMAC index is illegal.            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_emac_config ( BL_LILAC_RDD_EMAC_ID_DTE  xi_emac_id,
                                         RDD_RATE_LIMITER_ID_DTE   xi_rate_limiter_id,
                                         uint16_t                  xi_rate_shaper_max_burst );

#if !defined(LEGACY_RDP)
int rdd_lan_vport_cfg(rdd_vport_id_t port, rdd_rate_limiter_t rate_limiter);
#endif


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

/* stub function. used for G9991 */
void rdd_eth_tx_ddr_queue_addr_config ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                        BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                        uint32_t                   xi_ddr_address,
                                        uint16_t                   xi_queue_size,
										uint8_t                    xi_counter_id );

#if !defined(LEGACY_RDP)
int rdd_lan_vport_tx_queue_cfg(rdd_vport_id_t port, rdd_tx_queue_id_t queue_id, uint16_t packet_threshold,
    rdd_queue_profile_id_t profile_id);
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ds_tm_service_queue_config                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configure downstream ingress (service) queue           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures ingress queue:                                                */
/*   - the packet threshold indicates how many packets can be TX pending      */
/*     behind a ingress queue.                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_queue_id - ingress queue index                                        */
/*   xi_packet_threshold - ingress queue packet threshold. Overriden if WRED  */
/*   and/or drop precedence is used                                           */
/*   xi_rate_limiter_id - enable/disable rate limiter                         */
/*   xi_profile_id - profile ID. If WRED and drop precedence not used, should */
/*   be rdd_queue_profile_disabled. If enabled, xi_packet_threshold MUST be   */
/*   equal to minimum low threshold in profile.                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID - queue index is greater then 7    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_tm_service_queue_config ( BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                                        uint16_t                   xi_packet_threshold,
                                                        RDD_RATE_LIMITER_ID_DTE    xi_rate_limiter_id,
                                                        rdd_queue_profile          xi_profile_id );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_eth_tx_queue_flush                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - flush all the pending tx packets behind an EMAC.       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   flush all tx pending packets from an EMAC TX queue, due to a posible     */
/*   race condition, the task is performd by the Runner through a CPU message */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_emac_id - EMAC port index (ETH0 - ETH4)                               */
/*   xi_queue_id - ETH TX queue index                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_EMAC_ID - EMAC index is illegal.            */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID - queue index is greater then 7    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_eth_tx_queue_flush ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                                BL_LILAC_RDD_CPU_WAIT_DTE  xi_wait );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_eth_tx_queue_get_status                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_eth_tx_queue_get_status ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                     BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                                     rdpa_stat_1way_t           *stat );
#else
BL_LILAC_RDD_ERROR_DTE rdd_lan_vport_tx_queue_status_get ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                     BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                                     rdpa_stat_1way_t           *stat );
#endif


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function configures a queue profile. Each queue can be associated   */
/*   with such profile. Queue behaviour is set through xi_queue_profile       */
/*   parameters. By varying the parameters, it is possible to set one of the  */
/*   following  congestion control mehanisms:                                 */
/*                                                                            */
/*    * tail drop                                                             */
/*   xi_high_class.max_threshold == xi_low_class.max_threshold ==             */
/*   xi_high_class.min_threshold == xi_low_class.min_threshold                */
/*                                                                            */
/*    * tail drop with drop precedence                                        */
/*   xi_high_class.max_threshold == xi_high_class.min_threshold               */
/*   xi_low_class.max_threshold == xi_low_class.min_threshold                 */
/*                                                                            */
/*   * RED                                                                    */
/*   xi_high_class.max_threshold > xi_high_class.min_threshold                */
/*   xi_high_class.max_threshold == xi_low_class.max_threshold                */
/*   xi_high_class.min_threshold == xi_low_class.min_threshold                */
/*                                                                            */
/*   * RED with drop precedence                                               */
/*   xi_high_class.max_threshold > xi_high_class.min_threshold                */
/*   xi_low_class.max_threshold > xi_low_class.min_threshold                  */
/*                                                                            */
/*   Note that at all cases:                                                  */
/*   xi_high_class.max_threshold >= xi_high_class.min_threshold               */
/*   xi_low_class.max_threshold >= xi_low_class.min_threshold                 */
/*   xi_high_class.max_threshold >= xi_low_class.max_threshold                */
/*   xi_high_class.min_threshold >= xi_low_class.min_threshold                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_direction                                                             */
/*   xi_profile_id - sets one of rdd_queue_profile profile IDs                */
/*   xi_queue_profile:                                                        */
/*     xi_high_class - regular (non drop eligible) min/max thresholds         */
/*     xi_low_class - drop eligible min/max thresholds                        */
/*     xi_max_drop_probability (0-100) - probability above which all packets  */
/*     are dropped. input should be the relative value of probability , so    */
/*     85 means max probability 0.85; 100 means max probability 1             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_queue_profile_config ( rdpa_traffic_dir    xi_direction,
                                                  rdd_queue_profile   xi_profile_id,
                                                  RDD_QUEUE_PROFILE   *xi_queue_profile );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_quasi_policer_config                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - bridge configuration.                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures quasi policer for packets that were trapped     */
/*   since their source bridge port was not established yet. The policer      */
/*   get periodical budget, and when the budget exceeds, the packet is        */
/*   dropped otherwise the packet is forward as a normal packet.              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - the configured bridge port.                             */
/*   xi_allocated_budget - the allocated budget per bridge port in bytes per  */
/*   second                                                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_quasi_policer_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                     uint32_t                      xi_allocated_budget );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   bl_lilac_rdd_emac_rate_limiter_config                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - EMAC configuration.                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures downstream rate limiter for egress EMAC, the    */
/*   rate limiter get periodical budget, and when the budget exceeds, the     */
/*   EMAC does not transmit the packet. The budget is accumulated when not    */
/*   used until budget limit is reached.                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_rate_limiter_id - (0-5) or 14 for idle                                */
/*   xi_allocated_budget - the allocated budget per rate limiter in bytes per */
/*   second                                                                   */
/*   xi_budget_limit - bucket limit in allocation cycle. the limit budget per */
/*   rate limiter must be >= MTU, and larger than budget allocation in        */
/*   allocation cycle                                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_emac_rate_limiter_config ( RDD_RATE_LIMITER_ID_DTE  xi_rate_limiter_id,
                                                      RDD_RATE_LIMIT_PARAMS    *xi_budget );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_emac_queues_0_3_rate_limit_mode_config                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - LAN TX configuration.                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to overide EMAC TX queues 0-3 when EMAC */
/*   rate limiting is enabled                                                 */
/*   if enabled then all EMACs queues 0-3 on EMACs which participate EMAC     */
/*   rate limiting will override the rate limiter and forward with no regard  */
/*   to the budget of the rate limiter.                                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_emac_queues_0_3_mode - enable/disable the rate limiter overide.       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_emac_queues_0_3_rate_limit_mode_config ( BL_LILAC_RDD_FILTER_MODE_DTE  xi_emac_queues_0_3_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_overall_rate_limiter_config                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - WAN TX configuration.                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures upstream rate limiter for egress EMAC, the      */
/*   rate limiter get periodical budget, and when the budget exceeds, the     */
/*   WAN EMAC does not transmit the packet. The budget is accumulated when    */
/*   not used until budget limit is reached.                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_allocated_budget - the allocated budget in bytes per second           */
/*   xi_budget_limit - bucket limit in allocation cycle. the limit budget     */
/*   must be >= MTU, and larger than budget allocation in allocation cycle    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_overall_rate_limiter_config ( RDD_RATE_LIMIT_PARAMS  *xi_budget );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_mirroring_config                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - WAN RX and TX configuration.                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures mirroring of WAN ingress or egress data to      */
/*   an EMAC port for debugging.                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_mode - enable/disable mirroring                                */
/*   xi_emac_id - Mirroring port (ETH0 - ETH4)                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_wan_mirroring_config ( rdpa_traffic_dir              xi_direction,
                                                  BL_LILAC_RDD_FILTER_MODE_DTE  xi_filter_mode,
                                                  BL_LILAC_RDD_EMAC_ID_DTE      xi_emac_id );
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_sc_get                                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the operation                        */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_sc_get ( uint8_t  *xo_buffer,
                                    uint8_t  *xo_flag );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_drop_precedence_config                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_drop_precedence_config ( rdpa_traffic_dir  xi_direction,
                                                    uint16_t          xi_eligibility_vector );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_ingress_rate_limiter_config                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - configures upstream rate limiter per emac interface.   */
/*                     supports flow control action and drop action.          */
/*                     leaky bucket model (budget allocator drains the bucket */
/*                     acording to configured rate).                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the operation                        */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_emac_id -                                                             */
/*      lan0..lan4                                                            */
/*   xi_rate -                                                                */
/*      set rate limiter sustain rate in granularity of -                     */
/*      1000000/UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATOR_TIMER_INTERVAL */
/*      bytes/sec.                                                            */
/*      the default value is 5000 bytes/sec                                   */
/*      to disable the rate limiter set this parameter to - 0.                */
/*   xi_drop_limit -                                                          */
/*      effectively this parameter set leaky bucket size.                     */
/*      when number of tokens in bucket is larger than this threshold         */
/*      packets are discarded                                                 */
/*   xi_flow_control_threshold -                                              */
/*      pause frame is sent when number of tokens in bucket is greater than   */
/*      (xi_budget_limit - xi_flow_control_threshold).                        */
/*      pause duration time is equivalent to transmission time of             */
/*      (xi_budget_limit - xi_flow_control_threshold) bytes.                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none.                                                                    */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_us_ingress_rate_limiter_config ( BL_LILAC_RDD_EMAC_ID_DTE  xi_emac_id,
                                                            uint32_t                  xi_rate,
                                                            uint32_t                  xi_drop_threshold,
                                                            uint32_t                  xi_flow_control_threshold );
#endif


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function configures general policer parameters. When disabling all  */
/*   hooks to a policer, xi_commited_rate = 0 should be set.                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_direction                                                             */
/*   xi_policer_id                                                            */
/*   xi_commited_rate - policer commited information rate. Should be          */
/*   configured in bytes per second. Allowed Range is 1000Bps to              */
/*   311,000,000Bps.                                                          */
/*   xi_bucket_size - size of token bucket. This parameter defines tolerance  */
/*   to bursts. In each cycle (every 1ms) bucket is replensihed with commited */
/*   rate value divided by 1000, but to no more than bucket size. Allowed     */
/*   range is 8B to 311,000B. bucket size should be larger than commited rate */
/*   replensihed each cycle, thus xi_bucket_size * 1000 > xi_commited_rate    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_policer_config ( rdpa_traffic_dir             xi_direction,
                                            BL_LILAC_RDD_POLICER_ID_DTE  xi_policer_id,
                                            RDD_RATE_LIMIT_PARAMS        *xi_budget );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function returns general policer drop counter.                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_direction                                                             */
/*   xi_policer_id                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_drop_counter - per policer drop counter value                         */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_policer_drop_counter_get ( rdpa_traffic_dir             xi_direction,
                                                      BL_LILAC_RDD_POLICER_ID_DTE  xi_policer_id,
                                                      uint16_t                     *xo_drop_counter );
#endif


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


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function returns free pd pool sizes.                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_downstream_size                                                       */
/*   xo_upstream_size                                                         */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_free_packet_descriptors_pool_size_get ( uint32_t  *xo_downstream_size,
                                                                   uint32_t  *xo_upstream_size );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   bl_lilac_rdd_gpon_sniffer_get_head_pointer                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   gpon sniffer                                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns head pointer of gpon tx queue                      */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_tcont_id - T-CONT & Queue number to be configured                     */
/*   xi_rate_controller_id - Rate controller number to be configured          */
/*   xi_queue_id - GPON queue number to be configured                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_tx_queue_head_ptr - queue's head pointer                              */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_channel_sniffer_get_head_ptr ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                              BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                              BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                              uint32_t                             *xo_tx_queue_head_ptr );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   bl_lilac_rdd_gpon_sniffer_copy_packet                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   gpon sniffer                                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns head pointer of gpon tx queue                      */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_packet_descriptor_ptr     - pointer to packet descriptror             */
/*   xi_packet_buffer_ptr - pointer to buffer that will contain the copied    */
/*                          packet (must be at least 2KBytes)                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_packet_size - packet size                                             */
/*   xo_next_packet_descriptor_ptr - pointer to next packet in queue          */
/*                    (if 0 then this is the last packet in queue)            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_channel_sniffer_copy_packet ( uint32_t  xi_packet_descriptor_ptr,
                                                             uint8_t   *xi_packet_buffer_ptr,
                                                             uint32_t  *xo_packet_size,
                                                             uint32_t  *xo_next_packet_descriptor_ptr );


/******************************************************************************/
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures downstream ingress policers mode                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_policer_mode - rdd_single_priority_mode - traffic with any IC queue   */
/*                     id is dropped                                          */
/*                   - rdd_double_priority_mode - only traffic with IC queue  */
/*                     id 4-7 is dropped                                      */
/*                                                                            */
/******************************************************************************/
void rdd_ds_policers_mode_config ( rdd_policer_mode  xi_policer_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ds_service_queue_rate_limiter_config                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - DS TM ingress queues configuration.                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures downstream rate limiter for ingress queue EMAC, */
/*   the rate limiter get periodical budget, and when the budget exceeds, the */
/*   queue does not pass the PD to the egress queue. The budget is accumulated*/
/*   when not used until budget limit is reached.                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_rate_limiter_id - (6-13) or 14 for idle                               */
/*   xi_allocated_budget - the allocated budget per rate limiter in bytes per */
/*   second                                                                   */
/*   xi_budget_limit - bucket limit in allocation cycle. the limit budget per */
/*   rate limiter must be >= MTU, and larger than budget allocation in        */
/*   allocation cycle                                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_service_queue_rate_limiter_config ( RDD_RATE_LIMITER_ID_DTE  xi_rate_limiter_id,
                                                                  RDD_RATE_LIMIT_PARAMS    *xi_budget );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ds_service_queue_overall_rate_limiter_set                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - service queue overall rate limiter mode set            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the mode of overall rate limiter for the        */
/*   service queues. To enable the rate limiter, budget should be configured  */
/*   in function "rdd_ds_service_queue_rate_limiter_config".                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_overall_rate_limiter_mode - enable / disable                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_service_queue_overall_rate_limiter_set ( BL_LILAC_RDD_RATE_LIMITER_MODE_DTE  xi_overall_rate_limiter_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_service_queues_enable                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - global enable/disable                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   Global enable or disable of service queues                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   enable                                                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   return status                                                            */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_service_queues_enable ( uint32_t enable );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_tm_service_queues_initialize                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Inernal function                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   Initialize service queues memroy and descriptors                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   no input                                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE f_rdd_tm_service_queues_initialize ( void );


/******************************************************************************/
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function globally configures upstream rate controller sustain       */
/*      budget limit                                                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_sustain_budget_limit - threshold above which surplus sustain tokens   */
/*      are added to peak budget, in bytes. value should be smaller than      */
/*      65,536                                                                */
/*                                                                            */
/******************************************************************************/
void rdd_rate_controller_sustain_budget_limit_config ( uint32_t  xi_sustain_budget_limit );

/******************************************************************************/
/*     Dummy function for US PD offload support                               */
/******************************************************************************/
#if defined(LEGACY_RDP)
void rdd_wan_tx_ddr_queue_addr_config ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                        BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                        BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                        uint32_t                             xi_ddr_address,
                                        uint16_t                             xi_queue_size,
                                        uint8_t                              xi_counter_id );
#endif

BL_LILAC_RDD_ERROR_DTE rdd_eth_tx_queue_clear_stat ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                     BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id );

BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_clear_stat ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                     BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                     BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_queue_get_occupancy                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_get_occupancy ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                        BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                        BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                        uint32_t                             *xo_queue_occupancy );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_eth_tx_queue_get_occupancy                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_eth_tx_queue_get_occupancy ( BL_LILAC_RDD_EMAC_ID_DTE   xi_emac_id,
                                                        BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue_id,
                                                        uint32_t                   *xo_queue_occupancy );

#endif /* _BL_LILAC_DRV_RUNNER_TM_H */

