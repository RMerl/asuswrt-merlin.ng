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

#include "rdpa_mcast.h"
#include "rdpa_ip_class_basic.h"
#include "rdd_platform.h"

#define RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE  8

typedef struct natc_params
{
    rdpa_traffic_dir dir;
} natc_params_t;

typedef BL_LILAC_RDD_ADD_CONNECTION_DTE rdd_ip_flow_t;
typedef RDD_CONTEXT_ENTRY_UNION_DTS rdd_fc_context_t;

typedef struct {
    rdpa_mcast_flow_key_t key;
    rdd_fc_context_t context;
    uint32_t xo_entry_index;
} rdd_mcast_flow_t;

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
/*   add_connection - hold 5 tupples for the lookup key along with context    */
/*   direction - upstream or downstream                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_connection_entry_add ( rdd_ip_flow_t                    *add_connection,
                               rdpa_traffic_dir                 direction );

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
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_connection_entry_delete ( bdmf_index  entry_index );


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
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_connection_entry_search ( rdd_ip_flow_t  *get_connection,
                                  rdpa_traffic_dir                 direction,
                                  bdmf_index                       *entry_index );

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
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_connection_entry_get ( rdpa_traffic_dir    direction,
                               uint32_t            entry_index,
                               rdpa_ip_flow_key_t  *connection_entry,
                               bdmf_index            *context_index );


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
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_context_entry_get ( bdmf_index                  entry_index,
                            rdd_fc_context_t            *context_entry );


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
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_context_entry_flwstat_get ( bdmf_index                  entry_index,
                                    rdd_fc_context_t            *context_entry );

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
/*   ONLY USE THIS FUNCTION FOR MULTICAST CONTEXT MODIFICATIONS!              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_context_entry - a structure to hold the modified context entry data.  */
/*   xi_entry_index - the index of the entry to be modified.                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_context_entry_modify ( rdd_fc_context_t            *context_entry,
                               bdmf_index                  entry_index );


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
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_connections_number_get ( uint32_t  *connections_number );

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
/*     BDMF_ERR_...                                                           */
/*                                                                            */
/******************************************************************************/
int rdd_clear_connection_table ( void );

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
int rdd_3_tupples_connection_mode_config ( bdmf_boolean  tri_tuple_mode );


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
void rdd_full_flow_cache_acceleration_config ( rdd_full_fc_acceleration_mode  mode,
                                               rdpa_traffic_dir               direction,
                                               bdmf_boolean                   enable );

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
int rdd_fc_flow_ip_addresses_add ( RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *ip_addresses_entry,
                                   bdmf_index                          *entry_index,
                                   uint16_t                            *entry_sram_address );


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
int rdd_fc_flow_ip_addresses_get ( bdmf_index                          entry_index,
                                   RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *ip_addresses_entry,
                                   uint16_t                            *entry_sram_address );


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
int rdd_fc_flow_ip_addresses_delete_by_index ( bdmf_index entry_index );
int rdd_fc_flow_ip_addresses_delete_by_address ( RDD_FC_FLOW_IP_ADDRESSES_ENTRY_DTS  *ip_addresses_entry );


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
int rdd_ucast_ds_wan_udp_filter_get( bdmf_index                        entry_index,
                                     RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  *ds_wan_udp_filter_entry );


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
int rdd_ucast_ds_wan_udp_filter_add( RDD_DS_WAN_UDP_FILTER_ENTRY_DTS  *ds_wan_udp_filter_entry,
                                     bdmf_index                       *entry_index );


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
int rdd_ucast_ds_wan_udp_filter_delete( bdmf_index entry_index );


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
int rdd_fc_mcast_port_header_buffer_get( uint32_t index,
                                         RDD_FC_MCAST_PORT_HEADER_BUFFER_DTS *port_header_entry,
                                         uint8_t *port_header_buffer );


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
int rdd_fc_mcast_port_header_buffer_put( uint32_t index,
                                         uint8_t *port_header_buffer,
					 RDD_FC_MCAST_PORT_HEADER_BUFFER_DTS *port_header_entry );

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
/*     BDMF_ERR_....                                                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_mcast_connection_entry_add ( rdd_mcast_flow_t  *add_connection );


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
/*     BDMF_ERR_....                                                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_mcast_connection_entry_search ( rdd_mcast_flow_t  *get_connection,
                                           bdmf_index     *entry_index );

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
/*     BDMF_ERR_....                                                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_mcast_connection_entry_get ( uint32_t                 nat_cache_entry_index,
                                        bdmf_index               context_entry_index,
                                        rdpa_mcast_flow_key_t    *lookup_entry);

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
/*     BDMF_ERR_....                                                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_mcast_connection_entry_delete ( bdmf_index  context_entry_index );

BL_LILAC_RDD_ERROR_DTE f_rdd_connection_table_initialize ( void );

int rdd_nat_cache_submit_command( uint32_t command, 
                                  uint32_t *keyword, 
                                  uint32_t *hit_count, 
                                  uint32_t *byte_count);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_fc_global_cfg_get                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - gets the configured flow cache global configuration    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configured flow cache global configuration        */
/*   - packet acceleration mode: L3 only, L23 (L2 and L3).                    */
/*   - tcp priority ack flows enable: 1 (enable), 0 (disable)                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   global_cfg - configured flow cache global configuration                  */
/*                                                                            */
/*     BDMF_ERR_....                                                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_global_cfg_get( RDD_FC_GLOBAL_CFG_ENTRY_DTS *global_cfg );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_fc_global_cfg_set                                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - sets the flow cache global configuration               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the flow cache global configuration                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   global_cfg - new flow cache global configuration                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*                                                                            */
/*     BDMF_ERR_....                                                          */
/*                                                                            */
/******************************************************************************/
int rdd_fc_global_cfg_set( RDD_FC_GLOBAL_CFG_ENTRY_DTS *global_cfg );

int f_rdd_free_context_entry ( uint32_t  context_entry_index );
#endif

