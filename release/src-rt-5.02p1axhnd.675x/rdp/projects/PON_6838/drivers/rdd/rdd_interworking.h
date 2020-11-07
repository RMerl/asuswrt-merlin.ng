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

#ifndef _BL_LILAC_DRV_RUNNER_INTERWORKING_H
#define _BL_LILAC_DRV_RUNNER_INTERWORKING_H


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_cfg_add ( rdpa_traffic_dir                        xi_direction,
                                                                 int32_t                                 xi_rule_cfg_priority,
                                                                 rdpa_ic_type                            xi_rule_cfg_type,
                                                                 rdpa_ic_fields                          xi_rule_cfg_key_mask,
                                                                 rdpa_forward_action                     xi_rule_hit_action,
                                                                 rdpa_forward_action                     xi_rule_miss_action,
                                                                 rdd_ingress_classification_lookup_mode *xo_rule_cfg_lookup_mode,
                                                                 int generic_rule_cfg_idx1,
                                                                 int generic_rule_cfg_idx2 );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_cfg_delete ( rdpa_traffic_dir  xi_direction,
                                                                    int32_t           xi_rule_cfg_priority );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_cfg_modify ( rdpa_traffic_dir     xi_direction,
                                                                    int32_t              xi_rule_cfg_priority,
                                                                    rdpa_forward_action  xi_rule_hit_action,
                                                                    rdpa_forward_action  xi_rule_miss_action );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_table_print ( rdpa_traffic_dir  xi_direction );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_add ( rdpa_traffic_dir  xi_direction,
                                                             uint32_t          xi_rule_cfg_priority,
                                                             rdpa_ic_key_t     *xi_rule_key,
                                                             uint32_t          xi_context_id );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_delete ( rdpa_traffic_dir  xi_direction,
                                                                uint32_t          xi_rule_mask_priority,
                                                                rdpa_ic_key_t     *xi_rule_key );


BL_LILAC_RDD_ERROR_DTE rdd_us_ingress_classification_default_flows_config ( BL_LILAC_RDD_EMAC_ID_DTE      xi_emac_id,
                                                                            uint32_t                      xi_context_id );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_context_config ( rdpa_traffic_dir                            xi_direction,
                                                                   uint32_t                                    xi_context_id,
                                                                   const rdd_ingress_classification_context_t  *xi_context );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_context_get ( rdpa_traffic_dir                      xi_direction,
                                                                uint32_t                              xi_context_id,
                                                                rdd_ingress_classification_context_t  *xo_context );


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_context_counter_read ( rdpa_traffic_dir   xi_direction,
                                                                         uint8_t            xi_context_id,
                                                                         uint16_t           *xo_counter );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ds_wan_flow_config                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Configure downstream GEM flow                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures downstream GEM flow. 256 flows are supported    */
/*   by the GPON MAC, flow can be configured as a direct to the CPU (PLOAM,   */
/*   OMCI, direct CPU) or normal GEM flow that directed to the bridge for     */
/*   further classification                                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_downstream_gem_flow - downstream GEM flow index                       */
/*   xi_cpu_reason - PLOAM, OMCI or direct CPU, normal flow should be 0       */
/*   xi_flow_classify_mode - further Ehernet flow classification mode for     */
/*                           normal flows, packet based or GEM flow based     */
/*   xi_ingress_flow - downstream ingress flow when the previous              */
/*                     parameter is GEM flow based.                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_wan_flow_config ( uint32_t                                        xi_wan_flow,
                                                rdpa_cpu_reason                                 xi_cpu_reason,
                                                BL_LILAC_RDD_DOWNSTREAM_FLOW_CLASSIFY_MODE_DTE  xi_flow_classify_mode,
                                                uint8_t                                         xi_eth_flow );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_wan_dsl_bonding_config                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - upstream interface.                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function is not applicable for PON world but is a placeholder       */
/*   as RDPA invokves this.                                                   */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ptm_bonding - unused for xPON                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_OK - No error                                               */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_wan_dsl_bonding_config ( int                           xi_ptm_bonding) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_wan_flow_config                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - upstream interface.                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures upstream CAR GEM flow. 256 flows are supported  */
/*   by the bridge, the flow determine the TCONT, GEM port and CRC calc       */
/*   command for the MAC.                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_wan_flow - the upstream wan flow-id to be configured                  */
/*   xi_wan_channel - channel index from the packet will be transmitted.      */
/*   xi_hdr_type - Layer 2 header type mainly used in case of DSL modes.      */
/*   xi_wan_port - the GEM port-id with it the packet will be transmitted.    */
/*   xi_crc_calc - tell the GPON MAC to add or not CRC to the packet.         */
/*   xi_ptm_bonding - unused for xPON                                         */
/*   xi_pbits_to_queue_table_index - table index for pbits-to-queue mapping.  */
/*   xi_traffic_class_to_cqueue_table_index - table index for tc-to-queue     */
/*                                              mapping.                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*   BL_LILAC_RDD_OK - No error                                               */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_wan_flow_config ( uint32_t                      xi_wan_flow,
                                                RDD_WAN_CHANNEL_ID            xi_wan_channel,
                                                uint32_t                      xi_hdr_type,
                                                uint32_t                      xi_wan_port,
                                                BL_LILAC_RDD_TX_CRC_CALC_DTE  xi_crc_calc,
                                                int                           xi_ptm_bonding,
                                                uint8_t                       xi_pbits_to_queue_table_index,
                                                uint8_t                       xi_traffic_class_to_queue_table_index );


/******************************************************************************/
/*                                                                            */
/*   This function returns WAN flow entry parameters by WAN flow index        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_wan_flow - the upstream wan flow-id to be configured                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_wan_channel - channel index from the packet will be transmitted.      */
/*   xo_wan_port - the GEM port-id with it the packet will be transmitted.    */
/*   xo_crc_calc - tell the GPON MAC to add or not CRC to the packet.         */
/*   xo_pbits_to_queue_table_index - table index for pbits-to-queue mapping.  */
/*   xo_traffic_class_to_cqueue_table_index - table index for tc-to-queue     */
/*                                              mapping.                      */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_wan_flow_get ( uint32_t                      xi_wan_flow,
                                             RDD_WAN_CHANNEL_ID            *xo_wan_channel,
                                             uint32_t                      *xo_wan_port,
                                             BL_LILAC_RDD_TX_CRC_CALC_DTE  *xo_crc_calc,
                                             uint8_t                       *xo_pbits_to_queue_table_index,
                                             uint8_t                       *xo_traffic_class_to_queue_table_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_flow_classification_mode_config                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - upstream flow classification configuration             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the upstream flow classification mode for broadcom    */
/*   tag only. The mode is used by the firmware to build the key for the      */
/*   hash lookup.                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - LAN0-LAN4                                               */
/*   xi_flow_classify_mode - (vid, vid + pbits)/source port                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_flow_classification_mode_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE                  xi_bridge_port,
                                                                BL_LILAC_RDD_UPSTREAM_FLOW_CLASSIFY_MODE_DTE  xi_flow_classify_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ds_pbits_to_qos_entry_config                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - downstream interface (QOS mapping).                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures downstream tx queue for packet based QOS        */
/*   mapping. Tables are supported per bridge port and each table is indexed  */
/*   by 8 P-bits values.                                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - table index                                             */
/*   xi_pbits - P-bits value, an index to the table.                          */
/*   xi_bridge_port - downstream TX queue.                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_PBITS - illegal P-bits value (0-7)          */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_pbits_to_qos_entry_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                          uint32_t                      xi_pbits,
                                                          BL_LILAC_RDD_QUEUE_ID_DTE     xi_qos );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_pbits_to_qos_entry_config                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - upstream interface (QOS mapping).                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures upstream tx queue for packet based QOS mapping. */
/*   The table is indexed by 8 wan mappings and by 8 P-bits values.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_pbits - P-bits value, an index to the table.                          */
/*   xi_queue - upstream TX queue.                                            */
/*   xi_rate_controller - upstream rate controller index.                     */
/*   xi_wan_mapping_table_index - wan mapping.                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_PBITS - illegal P-bits value (0-7)          */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_pbits_to_qos_entry_config ( uint8_t                    xi_wan_mapping_table_index,
                                                          uint32_t                   xi_pbits,
                                                          BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue,
                                                          uint8_t                    xi_rate_controller );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_dscp_to_pbits_global_config                                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - bridge interface.                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures DSCP to P-bits conversion, only one global      */
/*   table is implemented for both downstream and upstream, the global table  */
/*   is used by non IP packets, for upstream GEM mapping and QOS mapping,     */
/*   and downstream QOS mapping.                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_dscp - the index to the table                                         */
/*   xi_pbits - the p-bits to be replaced or inserted                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_global_config ( uint32_t  xi_dscp,
                                                         uint32_t  xi_pbits );

BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_dei_global_config ( uint32_t  xi_dscp,
                                                             uint32_t  xi_pbits,
                                                             uint32_t  xi_dei );

void rdd_force_dscp_to_pbit_config(rdpa_traffic_dir dir, bdmf_boolean enable);

void rdd_rate_limit_overhead_cfg(uint8_t  xi_rate_limit_overhead);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ack_prioritization_config                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - TCP/ACK prioritization                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enable/disable ACK prioritization                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   enable - 0=disable , 1=enable                                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ack_prioritization_config(bdmf_boolean enable);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ack_packet_size_threshold_config                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - TCP/ACK prioritization                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the size threshold for ack packets to be checked in   */
/*   TCP/ACK prioritization feature                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   threshold (value: 0-255)                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ack_packet_size_threshold_config(uint8_t threshold);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_to_wan_us_ingress_flow_config                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - system debug routine                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures default upstream ingress flow for wan to wan    */
/*   packets ( WAN loopback ).                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ingress_flow - default wan to wan upstream ingress flow.              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_to_wan_us_ingress_flow_config ( uint32_t  xi_ingress_flow );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ds_traffic_class_to_queue_entry_config                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - downstream interface (QOS mapping).                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures downstream tx queue for packet based on         */
/*   traffic class. Tables are supported per bridge port and each table       */
/*   is indexed by 8 traffic class values.                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - table index                                             */
/*   xi_traffic_class - traffic class value, an index to the table.           */
/*   xi_queue - downstream TX queue.                                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BL_LILAC_RDD_ERROR_DTE - Return status                                 */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_TRAFFIC_CLASS - illegal traffic class       */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_traffic_class_to_queue_entry_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                    uint8_t                       xi_traffic_class,
                                                                    BL_LILAC_RDD_QUEUE_ID_DTE     xi_queue );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_traffic_class_to_queue_entry_config                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - upstream interface (QOS mapping).                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures upstream tx queue for packet based on           */
/*   traffic class. Tables are supported per wan mapping and each table       */
/*   is indexed by 16 traffic class values.                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_wan_mapping_table_index - wan mapping.                                */
/*   xi_traffic_class - traffic class value, an index to the table.           */
/*   xi_queue - upstream TX queue.                                            */
/*   xi_rate_controller - queue rate controller                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_OK - No error                                               */
/*   BL_LILAC_RDD_ERROR_ILLEGAL_WAN_MAPPING_TABLE_INDEX                       */
/*   BL_LILAC_RDD_ERROR_ILLEGAL_TRAFFIC_CLASS                                 */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_us_traffic_class_to_queue_entry_config ( uint8_t                    xi_wan_mapping_table_index,
                                                                    uint8_t                    xi_traffic_class,
                                                                    BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue,
                                                                    uint8_t                    xi_rate_controller );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_us_pbits_to_wan_flow_entry_config                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - upstream interface (pbit to wan flow mapping)          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures upstream wan flow                               */
/*   for packet based wan flow mapping.                                       */
/*   The table is indexed by 8 wan mappings and by 8 P-bits values.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_wan_mapping_table - wan mapping table (from ic context).              */
/*   xi_pbits - P-bits value, an index to the table.                          */
/*   xi_wan_flow - upstream wan flow.                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*   BL_LILAC_RDD_OK - No error                                               */
/*   BL_LILAC_RDD_ERROR_ILLEGAL_PBITS - illegal P-bits value (0-7)            */
/*   BL_LILAC_RDD_ERROR_ILLEGAL_PBITS_TO_WAN_FLOW_MAPPING_TABLE               */
/*                                                                            */
/*                                                                            */
/******************************************************************************/                                                                    
BL_LILAC_RDD_ERROR_DTE rdd_us_pbits_to_wan_flow_entry_config ( uint8_t  xi_wan_mapping_table,
                                                               uint8_t  xi_pbits,
                                                               uint8_t  xi_wan_flow );

#define NUM_OF_GENERIC_RULE_CFG 4
void rdd_ingress_classification_generic_rule_cfg(rdpa_traffic_dir dir,
    int gen_rule_cfg_idx, rdpa_ic_gen_rule_cfg_t *gen_rule_cfg);

#endif /* _BL_LILAC_DRV_RUNNER_INTERWORKING_H */

