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

#ifndef _BL_LILAC_DRV_RUNNER_ROUTER_H
#define _BL_LILAC_DRV_RUNNER_ROUTER_H

typedef struct {
    rdpa_mcast_flow_key_t key;
    RDD_CONTEXT_ENTRY_UNION_DTS context;
    uint32_t xo_entry_index;
} rdd_mcast_flow_t;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_subnet_classify_config                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Configure downstream subnet classification mode        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the subnet classification mode. 2 modes are     */
/*   supported:                                                               */
/*     1. downstream Ethernet flow                                            */
/*     2. destination mac filter - 4 filters are supported (0-3), should be   */
/*        configured in the IH HW                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_subnet_classify_mode - Ethernet flow based or MAC filter              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_subnet_classify_config ( BL_LILAC_RDD_SUBNET_CLASSIFY_MODE_DTE  xi_subnet_classify_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ipv6_config                                                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - router configuration.                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures whether to do connection search for IPV6        */
/*   packets, or to treat them as an IP header error packets and send them to */
/*   the layer 4 filters.                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ipv6_mode - enable or disable IPV6 connection search.                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ipv6_config ( BL_LILAC_RDD_IPV6_ENABLE_DTE  xi_ipv6_mode );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_connection_entry_add                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - add conncetion and context to the connection table.    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function add an entry to the connection table, the key for the hash */
/*   function is layer3 protocol, src & dst IP addresses and src & dst layer  */
/*   4 ports. Both connection and context tables are located in the DDR.      */
/*   The context table is based on dynamic allocation.                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_add_connection - hold 5 tupples for the lookup key along with context */
/*   xi_direction - upstream or downstream                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY - the table is full or MAX Hop     */
/*                                           has been reached.                */
/*     BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY - the context table is full.      */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_add ( BL_LILAC_RDD_ADD_CONNECTION_DTE  *xi_add_connection,
                                                  rdpa_traffic_dir                 xi_direction );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_connection_entry_delete                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - remove conncetion and context from the connection      */
/*                     table.                                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function remove an entry from the connection table by the index of  */
/*   the context entry. The context entry is free to the pool.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - the index of the entry to be deleted                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID - the context index is        */
/*                                                illegal.                    */
/*     BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY - the lookup entry is not valid */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_delete ( bdmf_index  xi_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_connection_entry_search                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get the index of a connection within the table.        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the index of an entry from the connection table.   */
/*   The key for the hash function is layer3 protocol, src & dst IP addresses */
/*   and src & dst layer 4 ports.                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_get_connection - hold 5 tupples for the lookup key                    */
/*   xi_direction - upstream or downstream                                    */
/*   xo_entry_index - the index of the entry in a case of a matched entry.    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY - the lookup key was not           */
/*                                              found in the table            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_search ( BL_LILAC_RDD_ADD_CONNECTION_DTE  *xi_get_connection,
                                                     rdpa_traffic_dir                 xi_direction,
                                                     bdmf_index                       *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_connection_entry_get                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - read connection according to index.                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_direction - upstream or downstream                                    */
/*   xi_entry_index - index in connection table                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_connection - connection entry (IPs are returned hashed in case        */
/*   connection is IPv6)                                                      */
/*   xo_context_index - context index                                         */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY - the lookup key was not           */
/*                                              found in the table            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_connection_entry_get ( rdpa_traffic_dir    xi_direction,
                                                  uint32_t            xi_entry_index,
                                                  rdpa_ip_flow_key_t  *xo_connection_entry,
                                                  bdmf_index          *xo_context_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_context_entry_get                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get context entry from the context table.              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function get a context entry from the context table by it's index.  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - the index of the entry to be read.                      */
/*   xo_context_entry - a structure to hold the context data                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID - the context index is        */
/*                                                illegal.                    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_context_entry_get ( bdmf_index                   xi_entry_index,
                                               RDD_CONTEXT_ENTRY_UNION_DTS  *xo_context_entry );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_context_entry_flwstat_get                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get context entry from the context table.              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function get a context entry with only the flow stats fields        */
/*    populated from the context table by it's index.                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - the index of the entry to be read.                      */
/*   xo_context_entry - a structure to hold the context data                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID - the context index is        */
/*                                                illegal.                    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_context_entry_flwstat_get ( uint32_t                     xi_entry_index,
                                                       RDD_CONTEXT_ENTRY_UNION_DTS  *xo_context_entry );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_context_entry_modify                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - modify context entry whithin the context table.        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function modifies a context entry whithin the context table by      */
/*   it's index.                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_context_entry - a structure to hold the modified context entry data.  */
/*   xi_entry_index - the index of the entry to be modified.                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID - the context index is        */
/*                                                illegal.                    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_context_entry_modify ( RDD_CONTEXT_ENTRY_UNION_DTS  *xi_context_entry,
                                                  bdmf_index                   xi_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_clean_context_aging_get                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get and aged context activity status.                  */
/*                                                                            */
/*   This function gets a context entry from the context table and claer its  */
/*   activity status bit for the aging process.                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - the index of the entry to be get and aged.              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_aging_status - hold the current acctivity status of the context.      */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_clean_context_aging_get ( uint32_t  xi_entry_index,
                                                     uint8_t   *xo_aging_status );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_connections_number_get                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - return the number of active connections in the system. */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the number of active connections whithin the       */
/*   system.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xo_connections_number -the number of active connections.                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_connections_number_get ( uint32_t  *xo_connections_number );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_clear_connection_table                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - delete all the connections from the connections table. */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function deletes all the connections from the connections table.    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none.                                                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_clear_connection_table ( void );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_dual_stack_lite_tunnel_config                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the source and destination IP addresses for     */
/*   each dual stack lite buffer (use in flow cache mode, otherwise set       */
/*   through rdd_dual_stack_lite_config)                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_dual_stack_lite_tunnel_id (0-3)                                       */
/*   xi_src_ip - source IPV6 for the tunneled packet.                         */
/*   xi_dst_ip - destination IPV6 for the tunneled packet.                    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_dual_stack_lite_tunnel_config ( rdpa_ds_lite_tunnel_id  xi_dual_stack_lite_tunnel_id,
                                                           bdmf_ipv6_t             *xi_ipv6_src_ip,
                                                           bdmf_ipv6_t             *xi_ipv6_dst_ip );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_dual_stack_lite_enable                                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   enables/disables stripping IPv6 header from IPv4 over IPv6 packets in DS */
/*   should be used only in flow cache (whne ds-lite not configured through   */
/*   subnet)                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ds_lite_enable                                                        */
/*                                                                            */
/******************************************************************************/
void rdd_dual_stack_lite_enable ( bdmf_boolean  xi_ds_lite_enable );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_layer4_filter_set                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - router configuration.                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures action for packets that were filtered by the    */
/*   hardware as an layer 4 pre defined or user defined protocols (see below  */
/*   xi_filter_index), action can be CPU trap or packet drop.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - ICMP, ICMPV6, IGMP, ESP, GRE, AH, and 4 user defined.  */
/*   xi_filter_action - CPU trap or packet drop.                              */
/*   xi_filter_parameter - CPU trap reason                                    */
/*   xi_direction - upstream or downstream                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_layer4_filter_set ( RDD_LAYER4_FILTER_INDEX                xi_filter_index,
                                               BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                               uint8_t                                xi_filter_parameter,
                                               rdpa_traffic_dir                       xi_direction );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_header_error_filter_config                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - router configuration.                                  */
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
/*   xi_filter_action - CPU trap or packet drop.                              */
/*   xi_filter_parameter - CPU trap reason                                    */
/*   xi_direction - upstream or downstream                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_header_error_filter_config ( BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                                        uint8_t                                xi_filter_parameter,
                                                        rdpa_traffic_dir                       xi_direction );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_ip_fragments_filter_config                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - router configuration.                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures action for IP fragmented packets. The actions   */
/*   can be CPU trap or packet drop.                                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_action - CPU trap or packet drop.                              */
/*   xi_filter_parameter - CPU trap reason                                    */
/*   xi_direction - upstream or downstream                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ip_fragments_filter_config( BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE  xi_filter_action,
                                                       uint8_t                                xi_filter_parameter,
                                                       rdpa_traffic_dir                       xi_direction );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function configures which action to take if connection search       */
/*   yields a miss. if filter is enabled, packet is trapped. otherwise,       */
/*   packet is forwarded to firewall or dropped.                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_enable - enable or disable filter                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ds_connection_miss_action_filter_config ( BL_LILAC_RDD_FILTER_MODE_DTE  xi_enable );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function writes rule index to firewall rules map table in entries   */
/*   [subnet_id][protocol][dst_port:dst_port_last], and writes source         */
/*   IP, source IP mask and dest IP to firewall rules table at the given      */
/*   rule index.                                                              */
/*   Note: When adding a new entry it shouldn't overlap existing entries      */
/*   (overlap means new.subnet_id = old.subnet_id; new.protocol =old.protocol */
/*   and new.dest_port_range intersects old.dest_port_range. It's OK to have  */
/*   new.dest_port_range == old.dest_port_range).                             */
/*                                                                            */
/* Input:                                                                     */
/*   RDD_FIREWALL_RULE_PARAMS                                                 */
/*   xi_rule_index - firewall rules table index (0-254)                       */
/*   xi_subnet_id (0-3)                                                       */
/*   xi_protocol (TCP/UDP)                                                    */
/*   xi_dst_port - first destination port in destination port range           */
/*   xi_dst_port_last - last destination port in destination port range       */
/*   *xi_src_ip - source ip. if none - should be NULL.                        */
/*   xi_src_ip_mask - source IP mask. number of 1s in mask from left. If none */
/*      should be > 31                                                        */
/*   *xi_dst_ip - destination ip. if none - should be NULL.                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_add ( uint32_t                  xi_rule_index,
                                               RDD_FIREWALL_RULE_PARAMS  *xi_firewall_rule_add );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function deletes rule from rules table, and updates 'next rule'     */
/*   field accordingly. If entry is first in rule chain, the function also    */
/*   updates rules map table with the next rule in chain (or with null, in    */
/*   case it's also the last rule in chain).                                  */
/*   Note: When removing an entry it should be consistent with an existing    */
/*   entry. In particular, part of port range can't be removed - it should be */
/*   the exact range corresponding to rule index.                             */
/*                                                                            */
/* Input:                                                                     */
/*   RDD_FIREWALL_RULE_PARAMS                                                 */
/*   rule_index - firewall rules table index (0-254)                          */
/*   subnet_id (0-3)                                                          */
/*   protocol (TCP/UDP)                                                       */
/*   dst_port - first destination port in destination port range              */
/*   dst_port_last - last destination port in destination port range          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_delete ( uint32_t                  xi_rule_index,
                                                  RDD_FIREWALL_RULE_PARAMS  *xi_firewall_rule_params );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function returns firewall rule entry fields of the map entry        */
/*   configured to the given dest port, protocol and subnet id.               */
/*                                                                            */
/* Input:                                                                     */
/*   RDD_FIREWALL_RULE_PARAMS                                                 */
/*   xi_subnet_id (0-3)                                                       */
/*   xi_protocol (TCP/UDP)                                                    */
/*   xi_dst_port - destination port                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   *xo_rule_index - firewall rules table index                              */
/*         RDD_FIREWALL_RULE_PARAMS                                           */
/*   check_mask_src_ip - mask source IP flag                                  */
/*   check_src_ip - check source IP flag                                      */
/*   check_dst_ip - check destination IP flag                                 */
/*   src_ip - source IP                                                       */
/*   src_ip_mask - source IP mask. number of 1s in mask from left             */
/*   dst_ip - destination IP                                                  */
/*   next_rule_index - next rule in rules chain                               */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_search ( uint32_t                  *xo_rule_index,
                                                  RDD_FIREWALL_RULE_PARAMS  *firewall_rule_params );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function returns firewall rule entry fields of the entry at         */
/*   the given index in firewall rules table.                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_rule_index - firewall rules table index (0-254)                       */
/*                                                                            */
/* Output:                                                                    */
/*   *RDD_FIREWALL_RULE_PARAMS                                                */
/*   check_mask_src_ip - mask source IP flag                                  */
/*   check_src_ip - check source IP flag                                      */
/*   check_dst_ip - check destination IP flag                                 */
/*   src_ip - source IP                                                       */
/*   src_ip_mask - source IP mask. number of 1s in mask from left             */
/*   dst_ip - destination IP                                                  */
/*   next_rule_index - next rule in rules chain                               */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_firewall_rule_get ( uint32_t                  xi_rule_index,
                                               RDD_FIREWALL_RULE_PARAMS  *xo_firewall_rule_params );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function controls whether ECN bits are remarked when DSCP           */
/*   remarking is enabled in router and packet is IPv6.                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_control - 0 - disabled, 1 - enabled                                   */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ipv6_ecn_remark ( uint32_t  xi_control );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function enable 3 tupple search for L3 IPv4/IPv6                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_3_tupple_enable - 0 - disabled, 1 - enabled                           */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_3_tupples_connection_mode_config ( bdmf_boolean  xi_3_tupple_mode );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function controls corner case handling in full flow cache mode      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_mode    - rdd_full_fc_acceleration_non_ip or                          */
/*                rdd_full_fc_acceleration_multicast_ip                       */
/*   xi_enable  - 0 - not accelerated (trapped as non tcp/udp)                */
/*              - 1 - accelerated in IC                                       */
/*                                                                            */
/******************************************************************************/
void rdd_full_flow_cache_acceleration_config ( rdd_full_fc_acceleration_mode  xi_mode,
                                               rdpa_traffic_dir               xi_direction,
                                               bdmf_boolean                   xi_enable );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function gets the current flow cache global configuration.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   None                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_global_cfg - flow cache global configuration                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_global_cfg_get(RDD_FC_GLOBAL_CFG_ENTRY_DTS *xo_global_cfg);


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function sets the current flow cache global configuration.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   None                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_global_cfg - flow cache global configuration                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_global_cfg_set(RDD_FC_GLOBAL_CFG_ENTRY_DTS *xi_global_cfg);

BL_LILAC_RDD_ERROR_DTE rdd_free_context_entry ( uint32_t  context_entry_index );

/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function adds an IP source and destination address entry to a       */
/*   table that is used for packet classification and modification.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ip_addresses_entry - IP source and destination addresses              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_entry_index - index of the table entry that is used for the entry     */
/*   xo_entry_sram_address - SRAM address of the table entry                  */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_add ( RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *xi_ip_addresses_entry,
                                                    bdmf_index                            *xo_entry_index,
                                                    uint16_t                              *xo_entry_sram_address );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function gets the IP source and destination address entry from      */
/*   a table.                                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xo_entry_index - index of the table entry to get                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xi_ip_addresses_entry - entry that contains IP source and destination    */
/*                         addresses                                          */
/*   xo_entry_sram_address - SRAM address of the table entry                  */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_get ( bdmf_index                        xi_entry_index,
                                                    RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *xo_ip_addresses_entry,
                                                    uint16_t                            *xo_entry_sram_address );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function deletes the IP source and destination address entry from   */
/*   a table.                                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_index - index of the array entry to get                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_delete_by_index ( bdmf_index xi_entry_index );
BL_LILAC_RDD_ERROR_DTE rdd_fc_flow_ip_addresses_delete_by_address ( RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *xi_ip_addresses_entry );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function gets the parameters of an active DS WAN UDP filter         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - index of the DS WAN UDP filter                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ds_wan_udp_filter_entry - DS WAN UDP filter parameters                */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ucast_ds_wan_udp_filter_get( bdmf_index                       xi_entry_index,
                                                        RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  *xo_ds_wan_udp_filter_entry );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function adds a new DS WAN UDP filter                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ds_wan_udp_filter_entry - DS WAN UDP filter parameters                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_entry_index - index of the new DS WAN UDP filter                      */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ucast_ds_wan_udp_filter_add( RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  *xi_ds_wan_udp_filter_entry,
                                                        bdmf_index                       *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function deletes an active DS WAN UDP filter                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - index of the DS WAN UDP filter to be deleted            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_ucast_ds_wan_udp_filter_delete( bdmf_index xi_entry_index );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function returns one entry from the multicast port headers buffer.  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_index - index of the array entry to put                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_port_header_buffer_get( uint32_t xi_index,
                                                            RDD_FC_MCAST_PORT_HEADER_BUFFER_DTS *xi_port_header_entry,
                                                            uint8_t *xo_port_header_buffer );


/******************************************************************************/
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This function sets one entry from the multicast port headers buffer.     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_index - index of the array entry to put                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_port_header_buffer_put( uint32_t xi_index,
                                                            uint8_t *xi_port_header_buffer,
                                                            RDD_FC_MCAST_PORT_HEADER_BUFFER_DTS *xo_port_header_entry );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_fc_mcast_connection_entry_add                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - adds mcast connection and context to the connection    */ 
/*   table.                                                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function add an entry to the connection table, the key for the hash */
/*   function is protocol, src & dst IP addresses, and number of tags.        */
/*   Both connection and context tables are located in the DDR. Mcast vlan    */
/*   table is located in SRAM. The context table is based on dynamic allocation.*/
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_add_connection - hold mcast tupple for the lookup key, and context    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY - the table is full or MAX Hop     */
/*                                           has been reached.                */
/*     BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY - the context table is full.      */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_add ( rdd_mcast_flow_t  *xi_add_connection );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_fc_mcast_connection_entry_search                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - gets the index of a connection within the table.       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the index of an entry from the connection table.   */
/*   The key for the hash protocol, src & dst IP addresses,  number of tags.  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_get_connection - hold mcast tupples for the lookup key                */
/*   xo_entry_index - the index of the entry in a case of a matched entry.    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY - the lookup key was not           */
/*                                              found in the table            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_search ( rdd_mcast_flow_t  *xi_get_connection,
                                                              bdmf_index        *xo_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_fc_mcast_connection_entry_get                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - reads connection according to index.                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_connection_entry_index - index in connection table                    */
/*   xi_context_entry_index    - index in context table                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_connection - connection entry (IPs are returned hashed in case        */
/*   connection is IPv6)                                                      */
/*   xo_context_index - context index                                         */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY - the lookup key was not           */
/*                                              found in the table            */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_get ( uint32_t               xi_connection_entry_index,
                                                           bdmf_index             xi_context_entry_index,
                                                           rdpa_mcast_flow_key_t  *xo_lookup_entry);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_fc_mcast_connection_entry_delete                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - removes connection and context from the connection     */
/*                     table.                                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function remove an entry from the connection table by the index of  */
/*   the context entry. The context entry is freed to the pool.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_entry_index - the index of the entry to be deleted                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_LILAC_RDD_ERROR_DTE - Return status                                   */
/*     BL_LILAC_RDD_OK - No error                                             */
/*     BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID - the context index is        */
/*                                                illegal.                    */
/*     BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY - the lookup entry is not valid */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_fc_mcast_connection_entry_delete ( bdmf_index  xi_context_entry_index );

#endif

