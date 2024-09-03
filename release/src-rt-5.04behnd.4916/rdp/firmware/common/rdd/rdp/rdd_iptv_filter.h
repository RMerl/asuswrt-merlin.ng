/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
:>
*/

#ifndef _DRV_RUNNER_IPTV_H
#define _DRV_RUNNER_IPTV_H

/* IPTV Tables */
#define RDD_IPTV_TABLE_SET_SIZE                       4
#define RDD_IPTV_ENTRY_SIZE                           16
#define RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS          32
#define RDD_IPTV_SOURCE_IP_TABLE_SEARCH_DEPTH         32
#define RDD_IPTV_LOOKUP_TABLE_CAM_SIZE                32
#define RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT              1024
#define RDD_IPTV_SSM_CONTEXT_TABLE_SIZE               (RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS * RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT)
#define RDD_IPTV_L3_SRC_IP_IPV6_SKIP_VALUE            0x8000
#define RDD_IPTV_L3_SRC_IP_IPV6_STOP_VALUE            0xFFFF
#define RDD_CONNECTION_TABLE_SET_SIZE    4

#define RDD_MAC_TABLE_CAM_SIZE 32

typedef struct
{
    uint16_t  entry[RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT];
} __PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONTEXT_TABLES_FREE_LIST_DTS;

#define RDD_CONTEXT_TABLES_FREE_LIST_ENTRY_READ(r, p)    MREAD_16(p, r)
#define RDD_CONTEXT_TABLES_FREE_LIST_ENTRY_WRITE(v, p)   MWRITE_16(p, v)

typedef struct iptv_params
{
    uint32_t iptv_table_size;   
} iptv_params_t;


int rdd_iptv_init(const rdd_module_t *module);
void rdd_iptv_lkp_method_cfg(rdpa_iptv_lookup_method iptv_mode);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_iptv_entry_add                                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Add an IPTV LUT entry                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function adds an entry to the IPTV lookup table. The search key is  */
/*   chosen according to the classification mode. The function returns the    */
/*   index of the entry in the LUT and an indication weather the entry        */
/*   resides in cache.                                                        */
/*   Notes:                                                                   */
/*       1) In iptv_lookup_method_group_ip_src_ip src_ip                      */
/*          should be set to 0.0.0.0 to denote "ANY"                          */
/*       2) In iptv_lookup_method_group_ip src_ip should                      */
/*          be set to 0.0.0.0 always.                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   entry - This paremeter can take  different forms according to the        */
/*           classification mode.                                             */
/*                                                                            */
/*              1. iptv_lookup_method_mac                                     */
/*				   mac_addr    - search key                                   */
/*   			   bridge_port - egress port                                  */
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                                                                            */                                                                                
/*              2. iptv_lookup_method_mac_vid                                 */
/*				   mac_addr    - search key                                   */
/*                 vid         - search key                                   */                                 
/*   			   bridge_port - egress port                                  */
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                                                                            */
/*              3. iptv_lookup_method_group_ip_src_ip                         */
/*				   dst_ip      - search key                                   */
/*                 src_ip      - search key                                   */
/*   			   bridge_port - egress port                                  */
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                                                                            */
/*              4. iptv_lookup_method_group_ip                                */
/*				   dst_ip      - search key                                   */
/*   			   bridge_port - egress port                                  */                     
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_index - index of the entry in LUT                                     */
/*   xo_cache - cached indication                                             */ 
/*                                                                            */
/*   int - Return status                                                      */
/*     0 - No error                                                           */
/*     BDMF_ERR_NO_MORE - the table is full or MAX Hop has been reached.      */
/*                                                                            */
/******************************************************************************/
int rdd_iptv_entry_add(rdd_iptv_entry_t *entry, uint32_t *xo_index, uint32_t *xo_cache);
int rdd_iptv_entry_modify(uint32_t entry_index, uint32_t egress_port_vector, uint16_t wifi_ssid_vector,
    uint8_t ingress_classification_context);
int rdd_iptv_entry_delete(uint32_t entry_index, uint32_t *xo_cache);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_iptv_entry_search                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Returns IPTV LUT entry index                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the index of an IPTV LUT entry. The search key is  */
/*   chosen according to the classification mode.                             */
/*   Notes:                                                                   */
/*       1) In iptv_lookup_method_group_ip_src_ip src_ip                      */
/*          should be set to 0.0.0.0 to denote "ANY"                          */
/*       2) In iptv_lookup_method_group_ip src_ip should                      */
/*          be set to 0.0.0.0 always.                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   entry - This paremeter can take  different forms according to the        */
/*           classification mode.                                             */
/*                                                                            */
/*              1. iptv_lookup_method_mac                                     */
/*				   mac_addr    - search key                                   */
/*                                                                            */                                                                                
/*              2. iptv_lookup_method_mac_vid                                 */
/*				   mac_addr    - search key                                   */
/*                 vid         - search key                                   */                                 
/*                                                                            */
/*              3. iptv_lookup_method_group_ip_src_ip                         */
/*				   dst_ip      - search key                                   */
/*                 src_ip      - search key                                   */
/*                                                                            */
/*              4. iptv_lookup_method_group_ip                                */
/*				   dst_ip      - search key                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_index - index of the entry in LUT                                     */
/*                                                                            */
/*   int - Return status                                                      */
/*     0 - No error                                                           */
/*     BDMF_ERR_NOENT - the lookup key was not found in the table             */
/*                                                                            */
/******************************************************************************/
int rdd_iptv_entry_search(rdpa_iptv_channel_key_t *, uint32_t *xo_index);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_iptv_entry_get                                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Firmware Driver - Retrieve IPTV LUT entry                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function retrieves a IPTV LUT entry according to its index.         */
/*   Notes:                                                                   */
/*       1) In iptv_lookup_method_group_ip_src_ip src_ip                      */
/*          will be set to 0.0.0.0 to denote "ANY"                            */
/*       2) In iptv_lookup_method_group_ip src_ip will be                     */
/*          set to 0.0.0.0 always.                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   entry_index -  index of the entry in LUT                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_entry - This paremeter can take  different forms according to the     */
/*           classification mode.                                             */
/*                                                                            */
/*              1. iptv_lookup_method_mac                                     */
/*				   mac_addr    - search key                                   */
/*   			   bridge_port - egress port                                  */
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                 counter     - entry packet counter                         */
/*                                                                            */                                                                                
/*              2. iptv_lookup_method_mac_vid                                 */
/*				   mac_addr    - search key                                   */
/*                 vid         - search key                                   */                                 
/*   			   bridge_port - egress port                                  */
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                 counter     - entry packet counter                         */
/*                                                                            */
/*              3. iptv_lookup_method_group_ip_src_ip                         */
/*				   dst_ip      - search key                                   */
/*                 src_ip      - search key                                   */
/*   			   bridge_port - egress port                                  */
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                 counter     - entry packet counter                         */
/*                                                                            */
/*              4. iptv_lookup_method_group_ip                                */
/*				   dst_ip      - search key                                   */
/*   			   bridge_port - egress port                                  */                     
/*                 ssid_vector - service set id vector for PCI with wifi      */
/*                               multiple ssid support.                       */
/*                 counter     - entry packet counter                         */
/*                                                                            */
/*     int - Return status                                                    */
/*     0 - No error                                                           */
/*     BDMF_ERR_NOENT - the entry index is not whithin legal table borders    */
/*                                                                            */
/******************************************************************************/
int rdd_iptv_entry_get(uint32_t entry_index, rdd_iptv_entry_t *xo_entry);
int rdd_iptv_counter_get(uint32_t entry_index, uint16_t *xo_counter_value);

#endif /* _DRV_RUNNER_IPTV_H */

