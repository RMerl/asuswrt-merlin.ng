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

#ifndef _BL_LILAC_DRV_RUNNER_L2_UCAST_H
#define _BL_LILAC_DRV_RUNNER_L2_UCAST_H

typedef BL_LILAC_RDD_ADD_CONNECTION_DTE rdd_l2_flow_t;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l2_connection_entry_add                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - add conncetion and context to the connection table.    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function add an entry to the connection table, the key for the hash */
/*   function is src and dst mac, eth type, # of VLAN tags, VLAN tags         */
/*   Both connection and context tables are located in the DDR.               */
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
int rdd_l2_connection_entry_add ( rdd_l2_flow_t     *add_connection,
                                  rdpa_traffic_dir  direction );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l2_connection_entry_delete                                           */
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
int rdd_l2_connection_entry_delete ( bdmf_index  flow_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l2_connection_entry_search                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - get the index of a connection within the table.        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the index of an entry from the connection table.   */
/*   The key for the hash function is src and dst mac, eth type,              */
/*   # of VLAN tags, VLAN tags.                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_get_connection - hold tupple for the lookup key                       */
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
int rdd_l2_connection_entry_search ( rdd_l2_flow_t          *get_connection,
                                     rdpa_traffic_dir       direction,
                                     bdmf_index             *entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l2_connection_entry_get                                                 */
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
int rdd_l2_connection_entry_get ( rdpa_traffic_dir    direction,
                                  uint32_t            entry_index,
                                  rdpa_l2_flow_key_t  *nat_cache_lkp_entry,
                                  bdmf_index          *flow_entry_index );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l2_context_entry_get                                                 */
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
int rdd_l2_context_entry_get ( bdmf_index                  flow_entry_index,
                               rdd_fc_context_t            *context_entry );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_l2_context_entry_modify                                              */
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
int rdd_l2_context_entry_modify ( rdd_fc_context_t *context_entry,
                                  bdmf_index       flow_entry_index );


int rdd_l2_context_entry_flwstat_get ( bdmf_index                  flow_entry_index,
                                       rdd_fc_context_t            *context_entry );
#endif

