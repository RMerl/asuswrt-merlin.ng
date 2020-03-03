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

#ifndef _BL_LILAC_DRV_RUNNER_BRIDGE_H
#define _BL_LILAC_DRV_RUNNER_BRIDGE_H



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_vlan_command_config                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Configure upstream vlan command                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures upstream vlan command action along with         */
/*   command parameters: 2 vlans and 2 P-bits values, the 2nd parametrs is    */
/*   the outer VLAN/P-bits while the 1st parameter is the inner               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_vlan_command_params - VLAN command parameters                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_config ( rdpa_traffic_dir                        xi_direction,
                                                 rdd_vlan_command_params                 *xi_vlan_command_params ); 


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function configures whether US VLAN aggregation                     */
/*   is enabled per lan port                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port (only emac0 to emac4)                                     */
/*   xi_vlan_aggregation_mode (enabled/disabled) - only when vlan aggregation */
/*                                                 is enabled                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_us_vlan_aggregation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE       xi_bridge_port,
                                                        BL_LILAC_RDD_AGGREGATION_MODE_DTE  xi_vlan_aggregation_mode );
#endif


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function globally configures whether VLAN switching (aggregation    */
/*   and isolation) is enabled                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_vlan_switching_mode (enabled/disabled)                                */
/*   xi_vlan_binding_mode (enabled/disabled) - only when vlan switching is    */
/*                                             enabled                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_config ( BL_LILAC_RDD_VLAN_SWITCHING_CONFIG_DTE  xi_vlan_switching_mode,
                                                   BL_LILAC_RDD_VLAN_BINDING_CONFIG_DTE    xi_vlan_binding_mode );
#endif


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function adds an entry to LAN VID CAM table, and sets the           */
/*   corresponding context                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_vid (0-4095)                                                          */
/*   xi_bridge_port_vector - bit vector of all eligible LAN ports for VLAN    */
/*   isolation.                                                               */
/*   xi_aggregation_mode (enable/disable)                                     */
/*   xi_aggregation_vid_index (0-3) - index of WAN VID in WAN VID table       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   *xo_entry_index - index where entry was added in LAN VID table           */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_add ( RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr,
                                               uint32_t            *xo_entry_index );
#endif


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function deletes an entry from LAN VID CAM table by marking it as   */
/*   'skip' (0x8000)                                                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_vid (0-4095) - VID to be deleted                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_delete ( uint32_t  xi_entry_index );
#endif


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function modifies the context of an entry in LAN VID table          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_vid (0-4095) - VID's context to be modified                           */
/*   xi_bridge_port_vector - bit vector of all eligible LAN ports for VLAN    */
/*   isolation.                                                               */
/*   xi_aggregation_mode (enable/disable)                                     */
/*   xi_aggregation_vid_index (0-3) - index of WAN VID in WAN VID table       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_modify ( uint32_t            xi_entry_index,
                                                  RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function configures one of four WAN VIDs for VLAN aggregation       */
/*   feature                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_wan_vid_index (0-3) - index of WAN VID to be inserted                 */
/*   xi_wan_vid (0-4095) - WAN VID to be inserted                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_vid_config ( uint8_t   xi_wan_vid_index,
                                            uint16_t  xi_wan_vid );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function configures whether to commit vlan isolation on a certain   */
/*   bridge port, configuration is seperate for the bridge port when acts as  */
/*   an ingress or egress                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - the configured bridge port                              */
/*   xi_direction - downstream is for egress and upstream is for ingress      */
/*   xi_vlan_switching_isolation__mode - enable/disable isolatio              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_isolation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             rdpa_traffic_dir              xi_direction,
                                                             BL_LILAC_RDD_FILTER_MODE_DTE  xi_vlan_switching_isolation_mode );
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_dscp_to_pbits_config                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - bridge interface.                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures DSCP to P-bits conversion, tables are           */
/*   implemented per bridge port, these tables are used by non IP packets,    */
/*   for both downstream and upstream VLAN action of P-bits command.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - bridge port index, destinguish between tables.          */
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
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  uint32_t                      xi_dscp,
                                                  uint32_t                      xi_pbits );
#else
void rdd_dscp_to_pbits_cfg(rdpa_traffic_dir direction, rdd_vport_id_t vport_id, uint32_t dscp, uint32_t pbits);
#endif

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_pbits_to_pbits_config                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - VLAN action configuration.                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the Pbits to Pbits table which is used to VLAN  */
/*   action with Pbits remark.                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_number - four remapping tables are available                    */
/*   xi_input_pbits - the index to the table                                  */
/*   xi_output_pbits - the P-bits bits to be replaced                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_pbits_to_pbits_config ( uint32_t  xi_table_number,
                                                   uint32_t  xi_input_pbits,
                                                   uint32_t  xi_output_pbits ); 
#else
int rdd_pbits_to_pbits_config ( uint32_t  xi_table_number,
                                uint32_t  xi_input_pbits,
                                uint32_t  xi_output_pbits );
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_tpid_overwrite_table_config                                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - VLAN action configuration.                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the TPID values for VLAN actions in which the   */
/*   TPID should be overwrite, 8 values are configured at the same time.      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   tpid_overwrite_array - 8 TPID values array                               */
/*   xi_direction - upstream or downstream                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_tpid_overwrite_table_config ( uint16_t          *tpid_overwrite_array,
                                                         rdpa_traffic_dir  xi_direction );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_tpid_detect_filter_value_config                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the TPID value for the TPID Detect filter       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_direction - upstream or downstream                                    */
/*   tpid_detect_filter_value - the TPID value                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_value_config ( rdpa_traffic_dir  xi_direction,
                                                             uint16_t          tpid_detect_filter_value );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_tpid_detect_filter_config                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to check that outer TPID equals a       */
/*   the configured value.                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_tpid_filter_mode - enable or disable the filter                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                       BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                       BL_LILAC_RDD_FILTER_MODE_DTE  xi_tpid_filter_mode );
#else
int rdd_tpid_detect_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_dhcp_filter_config                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap dhcp packes to the cpu.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_dhcp_filter_mode - enable or disable the filter                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_dhcp_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                BL_LILAC_RDD_FILTER_MODE_DTE  xi_dhcp_filter_mode );
#else
int rdd_dhcp_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*  rdd_mld_filter_config                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap MLD packes to the cpu.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_mld_filter_mode - enable or disable the filter                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_mld_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                               BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                               BL_LILAC_RDD_FILTER_MODE_DTE  xi_mld_filter_mode );
#else
int rdd_mld_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*  rdd_1588_filter_config                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap 1588 packets to the cpu.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_1588_filter_mode - enable or disable the filter                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_1588_layer4_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                       BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                       BL_LILAC_RDD_FILTER_MODE_DTE  xi_1588_filter_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_igmp_filter_config                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap IGMP packes to the cpu or to    */
/*   drop them.                                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_igmp_filter_mode - enable or disable the filter                       */
/*   xi_filter_action - CPU trap or drop                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_igmp_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_igmp_filter_mode,
                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action );
#else
int rdd_igmp_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir,
    bdmf_boolean enable, rdd_action filter_action, bdmf_boolean local_switch_en);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_icmpv6_filter_config                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap ICMPv6 packes to the cpu.       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_icmpv6_filter_mode - enable or disable the filter                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_icmpv6_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                  BL_LILAC_RDD_FILTER_MODE_DTE  xi_icmpv6_filter_mode );
#else
int rdd_icmpv6_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir,
    bdmf_boolean enable, bdmf_boolean local_switch_en);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ether_type_filter_config                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap packes with certain ether type  */
/*   to the cpu or to drop them.                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_ether_type_filter_mode - enable or disable the filter                 */
/*   xi_ether_type_filter_number - 8 filters are available, 4 static like IP  */
/*          or PPPOE or 4 user defined are configurable through IH driver     */
/*   xi_ether_type_action - CPU trap or drop                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_ether_type_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE               xi_bridge_port,
                                                      BL_LILAC_RDD_SUBNET_ID_DTE                 xi_subnet_id,
                                                      BL_LILAC_RDD_FILTER_MODE_DTE               xi_ether_type_filter_mode,
                                                      BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE  xi_ether_type_filter_number,
                                                      BL_LILAC_RDD_FILTER_ACTION_DTE             xi_ether_type_action );
#else
int rdd_ether_type_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    uint8_t ether_type_filter_num, rdd_action filter_action, bdmf_boolean local_switch_en);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_broadcast_filter_config                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap broadcast packes to the cpu     */
/*   or to drop them.                                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_broadcast_filter_mode - enable or disable the filter                  */
/*   xi_filter_action - CPU trap or drop                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_broadcast_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                     BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                     BL_LILAC_RDD_FILTER_MODE_DTE    xi_broadcast_filter_mode,
                                                     BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action );
#else
int rdd_bcast_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_multicast_filter_config                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to trap multicast packes to the cpu     */
/*   or to drop them.                                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_multicast_filter_mode - enable or disable the filter                  */
/*   xi_filter_action - CPU trap or drop                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_multicast_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                     BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                     BL_LILAC_RDD_FILTER_MODE_DTE    xi_multicast_filter_mode,
                                                     BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action );
#else
int rdd_mcast_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_local_switching_filters_config                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures filters mode in local switching                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_filter_mode - enable or disable the filters                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID - illegal bridge port        */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_local_switching_filters_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                            BL_LILAC_RDD_FILTER_MODE_DTE  xi_local_switching_filters_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ip_fragments_ingress_filter_config                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures action for IP fragmented packets. The actions   */
/*   can be CPU trap or packet drop.                                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_ip_fragments_filter_mode - enable or disable the filter               */
/*   xi_filter_action - CPU trap or drop                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_ip_fragments_ingress_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_ip_fragments_filter_mode,
                                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action );
#else
int rdd_ip_fragments_ingress_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   bl_lilac_rdd_ip_fragments_ingress_filter_config                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Ingress filters configuration.                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures action for packets that were marked by hardware */
/*   as an error or exception, the reason for that can be IP checksum error,  */
/*   or any other error in the header, like IPV6 when IPV6 is disabled,       */
/*   action can be CPU trap or packet drop.                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - ingress filter is configured per bridge port            */
/*   xi_subnet_id - relevant for wan routed bridge port, otherwise 0          */
/*   xi_ip_fragments_filter_mode - enable or disable the filter               */
/*   xi_filter_action - CPU trap or drop                                      */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_header_error_ingress_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_header_error_filter_mode,
                                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action );
#else
int rdd_hdr_err_ingress_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_virtual_port_config                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - bridge interface.                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures whether Ethernet flows with direct forwarding enabled use     */
/*   egress port from flow, or derive egress port by hashing DA+SA (only      */
/*   downstream).                                                             */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_lan_port_vector - vector containing up to 4 ports that participate in */
/*   hash based forwarding. If more than 4 are configured, first 4 are used.  */
/*   If 0 is passed, default egress port in Ethernet flow is used.            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_virtual_port_config ( BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  xi_lan_port_vector );
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   brdd_ds_mac_unknown_da_forwarding_cfg                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Configure DS unknown DA forwarding entry               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures how an unknown da that matches a certain prefix */
/*   should be handled. Configuration conists of describing the egress        */
/*   port(s) and optional policer.                                            */
/*   The feature is disabled by default                                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_mac_prefix        - DA prefix ( disable with 0xFFFFFFF )              */
/*   xi_bridge_port       - egress port for prefix match packets              */
/*   xi_wifi_ssid_vector  - service set id vector for PCI with wifi multiple  */
/*                          ssid support.                                     */
/*   xi_policer_id        - genreal policer id                                */
/*                          ( disable with BL_LILAC_RDD_POLICER_DISABLED )    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_mac_unknown_da_forwarding_cfg ( uint32_t                      xi_mac_prefix,
                                                              BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                              uint16_t                      xi_wifi_ssid_vector,
                                                              BL_LILAC_RDD_POLICER_ID_DTE   xi_policer_id );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_upstream_unknown_da_flooding_bridge_port_config                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - bridge interface.                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures upstream unknown DA flooding destination port (WAN / CPU )    */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port: WAN or CPU bridge ports                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_upstream_unknown_da_flooding_bridge_port_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port );



/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_bridge_flooding_config                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - bridge interface.                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures DA MAC lookup failure action (forward, CPU trap, or drop      */
/*   packet.                                                                  */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_ports_vector - bridge ports vector for multicast flooding,     */
/*                            when unknown DA command is forward packet       */
/*   xi_wifi_ssid_vector  - service set id vector for PCI with wifi multiple  */
/*                          ssid support.                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_bridge_flooding_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_ports_vector,
                                                    uint16_t                      xi_wifi_ssid_vector );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mac_entry_add                                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - add MAC address to the MAC lookup table                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function add an entry to the MAC table, the key for the hash        */
/*   function is the MAC address, this table is used both for source and      */
/*   destination MAC lookups, therefore the bridge port is used .             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_mac_addr - MAC address as the lookup key                              */
/*   xi_bridge_port - bridge port from which the MAC is belong to.            */
/*   xi_aggregation_mode - enables/disables VLAN aggregation                  */
/*   xi_extension_entry - service set id for PCI with wifi multiple ssid      */
/*   support or VID index for aggregation if entry is aggregated              */
/*   xi_sa_action - source MAC action ( forward / Trap / Drop )               */
/*   xi_da_action - destination MAC action ( forward / Trap / Drop )          */
/*   xo_mac_entry_index - the entry index in case of success                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY - the table is full or MAX Hop     */
/*                                           has been reached.                */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_add ( RDD_MAC_PARAMS  *xi_mac_params_ptr,
                                           uint32_t        *xo_mac_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mac_entry_delete                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - delete MAC address from the MAC lookup table           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function deletes an entry from the MAC lookup table, the key for    */
/*   the hash function is the MAC address.                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_mac_addr - MAC address as the lookup key                              */
/*   xi_entry_type - static or bridge entry                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY - the lookup key was not        */
/*                                              found in the table            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_delete ( bdmf_mac_t                       *xi_mac_addr,
                                              BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE  xi_entry_type );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mac_entry_modify                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - modify MAC address within the MAC lookup table         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function modify an entry whithin the MAC lookup table, the key for  */
/*   the hash function is the MAC address, the parameters that can be changed */
/*   are the bridge port and the source/destination actions.                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_mac_addr - MAC address as the lookup key                              */
/*   xi_bridge_port - bridge port from which the MAC is belong to.            */
/*   xi_wifi_ssid - service set id for PCI with wifi multiple ssid support    */
/*   xi_sa_action - source MAC action ( forward / Trap / Drop )               */
/*   xi_da_action - destination MAC action ( forward / Trap / Drop )          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY - the lookup key was not */
/*                                                     found in the table     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_modify ( RDD_MAC_PARAMS  *xi_mac_params_ptr );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mac_entry_search                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - return MAC address index within the MAC lookup table   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function return the entry index of a MAC entry within the lookup    */
/*   table, the key for the hash function is the MAC address.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_mac_addr - MAC address as the lookup key.                             */
/*   xo_entry_index - entry index within the MAC lookup table.                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY - the lookup key was not */
/*                                                     found in the table     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_search ( RDD_MAC_PARAMS  *xi_mac_params_ptr,
                                              uint32_t        *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mac_entry_get                                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - retreive information from MAC lookup table             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function retreive information of a MAC entry within the MAC lookup  */
/*   table, according to entry index input parameter.                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index -  entry index within the MAC lookup table.               */
/*   xo_mac_addr - MAC address.                                               */
/*   xo_bridge_port - bridge port.                                            */
/*   xo_entry_type - static or bridge entry                                   */
/*   xo_aggregation_mode - enable/disable of vlan switching aggregation       */
/*   xo_extension_entry - if bridge port is PCI than contains service set id  */
/*   for PCI with wifi multiple ssid support. If xo_aggregation_mode is       */
/*   enabled then contains lan vid table index for aggregation                */
/*   xo_sa_action - source MAC action ( forward / Trap / Drop )               */
/*   xo_da_action - destination MAC action ( forward / Trap / Drop )          */
/*   xo_valid_bit - indication whether the entry is valid                     */
/*   xo_skip_bit - indication whether the entry should be skip                */
/*   xo_aging_bit - indication whether the entry should be age                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_MAC_ENTRY_ID - the entry index is not       */
/*                                               whithin legal table borders  */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_get ( uint32_t        xi_entry_index,
                                           RDD_MAC_PARAMS  *xo_mac_params_ptr,
                                           uint32_t        *xo_valid_bit,
                                           uint32_t        *xo_skip_bit,
                                           uint32_t        *xo_aging_bit );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_mac_entry_aging_set                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - This API set the aging control bit of a MAC entry      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   aging is done in a priodical cycle, a valid entry with aging bit cleared */
/*   is enter into a "not used" state and its aging bit is set, in the next   */
/*   cycle if the aging bit is still set the entry is deleted, aging bit can  */
/*   be laso cleared by the hardware when the MAC lookup key is encountered   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - entry index within the MAC lookup table.                */
/*   xo_activity_status - current activity status                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ILLEGAL_MAC_ENTRY_ID - the entry index is not       */
/*                                               whithin legal table borders  */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_aging_set ( uint32_t  xi_entry_index,
                                                 uint32_t  *xo_activity_status );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_clear_mac_table                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - delete all the MAC addresses from the MAC lookup table */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This API simply clear and delete all the MAC addresses from the MAC      */
/*   lookup table.                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   None.                                                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_clear_mac_table ( void );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_forwarding_matrix_config                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - bridge configuration                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures forwarding (eligibility) matrix between sourec port and       */
/*   destination port of the packet, this API should be configured along with */
/*   the IH forwarding matrix, and it is used in cases that the hardware does */
/*   not support, like destination lookup failure, or routed packets.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_src_bridge_port - source bridge port.                                 */
/*   xi_dst_bridge_port - destination bridge port.                            */
/*   xi_enable          - enables or disables the forwarding.                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_forwarding_matrix_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_src_bridge_port,
                                                      BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_dst_bridge_port,
                                                      BL_LILAC_RDD_FORWARD_MATRIX_DTE  xi_enable );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wifi_ssid_forwarding_matrix_config                                   */
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
BL_LILAC_RDD_ERROR_DTE rdd_wifi_ssid_forwarding_matrix_config ( uint16_t                      xi_wifi_ssid_vector,
                                                                BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_dst_bridge_port );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_egress_ethertype_config                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - VLAN configuration                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures ether types for add single VLAN and add double VLAN comands.  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_1st_ether_type - ether type value for 1st ether type.                 */
/*   xi_2nd_ether_type - ether type value for 2nd ether type.                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_egress_ethertype_config ( uint16_t  xi_1st_ether_type,
                                                     uint16_t  xi_2nd_ether_type );
#else
void rdd_egress_ethertype_config ( uint16_t  xi_1st_ether_type,
                                   uint16_t  xi_2nd_ether_type );
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*  rdd_vlan_command_always_egress_ether_type_config                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - VLAN configuration                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures ether type for add vlan always command.                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_3rd_ether_type - ether type value.                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_always_egress_ether_type_config ( uint16_t  xi_3rd_ether_type );
#else
void rdd_vlan_command_always_egress_ether_type_config ( uint16_t  xi_3rd_ether_type );
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_src_mac_anti_spoofing_lookup_config                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - defines whether to activate OUI filter                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether OUI lookup should be set for that       */
/*   bridge port                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - bridge port filter configuration                        */
/*   xi_filter_mode -                                                         */
/*          BL_LILAC_RDD_FILTER_DISABLE /                                     */
/*          BL_LILAC_RDD_FILTER_ENABLE                                        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
#ifdef LEGACY_RDP
BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_lookup_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                 BL_LILAC_RDD_FILTER_MODE_DTE  xi_filter_mode );
#else
int rdd_src_mac_anti_spoofing_lookup_cfg(rdd_port_profile_t profile_idx, bdmf_boolean enable);
#endif


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_src_mac_anti_spoofing_entry_add                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - add an entry to the source MAC anti spoofing table     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function add an entry to the source MAC anti spoofing CAM lookup.   */
/*   The lookup key is source MAC prefix ( 3 MSB ). 4 entries can be          */
/*   configured per bridge port.                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - bridge port for table selection                         */
/*   xi_src_mac_prefix - source MAC address prefix ( 3 MSB )                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CAM_LOOKUP_TABLE_FULL - the lookup table is full.   */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_add ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             uint32_t                      xi_src_mac_prefix );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_src_mac_anti_spoofing_entry_delete                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - delete an entry from the source MAC anti spoofing      */
/*                     table                                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function delete an entry from the source MAC anti spoofing CAM      */
/*   lookup table. The lookup key is source MAC prefix ( 3 MSB ).             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bridge_port - bridge port for table selection                         */
/*   xi_src_mac_prefix - source MAC address prefix ( 3 MSB )                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CAM_LOOKUP_FAILED - the entry was not found in the  */
/*                                            CAM lookup table.               */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_delete  ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                 uint32_t                      xi_src_mac_prefix );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_1588_master_rx_get_entry				                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get data from 1588_rx_master table                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function looks for an entry in the 1588 RX master table according   */
/*		to the entry index                                                    */
/*		    							                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index 		- 1588 table entry index                          */
/*                                                                            */
/* Output:                                                                    */
/*   xo_tod_high - timestamp high                                             */
/*   xo_tod_low  - timestamp low                                              */  
/*   xo_local_clock_delta - local clock delta (RUNNER_CLK - BBH_CLK = delta)  */	    
/*	                                                                          */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*   BL_LILAC_RDD_OK - No error                                               */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_1588_master_rx_get_entry ( uint16_t  xi_entry_index,
                                                      uint32_t  *xo_tod_high,
                                                      uint32_t  *xo_tod_low,
                                                      uint16_t  *xo_local_clock_delta );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_lag_port_get           				                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get LAG port                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function calculates LAG port according source and dest MAC address  */
/*		    							                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_addr 	- packet source MAC address                           */
/*   xi_dest_addr 		- packet destination MAC address                      */
/*                                                                            */
/* Output:                                                                    */
/*   xo_egress_port - calculated egress port                                  */	    
/*	                                                                          */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*   BL_LILAC_RDD_OK - No error                                               */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static inline BL_LILAC_RDD_ERROR_DTE rdd_lag_port_get ( uint8_t	                        *xi_source_addr, 
                                                        uint8_t	                        *xi_dest_addr,
                                                        BL_LILAC_RDD_BRIDGE_PORT_DTE	*xo_egress_port )
{
    uint8_t                        *hash_buffer_ptr;
    unsigned long                  flags;
    BL_LILAC_RDD_ERROR_DTE         rdd_error;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    hash_buffer_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );
        
    MWRITE_BLK_8( hash_buffer_ptr, xi_source_addr + 4, 2 );
    MWRITE_BLK_8( hash_buffer_ptr + 2, xi_dest_addr, MAC_ADDRESS_SIZE );
    MWRITE_BLK_8( hash_buffer_ptr + MAC_ADDRESS_SIZE + 2, xi_source_addr, 4 );

    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_LAG_PORT_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

    *xo_egress_port = ( BL_LILAC_RDD_BRIDGE_PORT_DTE ) *( volatile uint32_t * )hash_buffer_ptr;

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );

    return  rdd_error;
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_lan_get_stats                                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get LAN port stats                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the tx valid and discard packet counts of the LAN  */
/*   port.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_lan_port 	- lan port [0-6].                                           */
/*                                                                            */
/* Output:                                                                    */
/*   rx_packet    - rx valid packet count                                     */
/*   tx_packet    - tx valid packet count                                     */
/*   tx_discard   - tx discard packet count                                   */
/*	                                                                           */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*   BL_LILAC_RDD_OK - No error                                               */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_lan_get_stats ( uint32_t   xi_lan_port,
                                           uint32_t   *rx_packet,
                                           uint32_t   *tx_packet,
                                           uint16_t   *tx_discard );

#endif /* _BL_LILAC_DRV_RUNNER_BRIDGE_H */

