/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#ifndef _RDD_INGRESS_FILTERS_H
#define _RDD_INGRESS_FILTERS_H

#define RDD_INVALID_PORT_PROFILE_VAL      255

typedef struct ingress_filters_params
{
    uint32_t filters_config_table_addr;
    uint32_t lkp_table_addr;
    uint32_t parameter_table_addr;
    rdpa_traffic_dir dir;
} rdd_if_params_t;

int rdd_ingress_filters_init(const rdd_module_t *module);
int rdd_ip_address_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable);
int rdd_dhcp_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en);
int rdd_mld_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en);
int rdd_1588_layer4_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable);
int rdd_igmp_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en);
int rdd_icmpv6_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en);
int rdd_ether_type_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    uint8_t ether_type_filter_num, rdd_action filter_action, bdmf_boolean local_switch_en);
int rdd_bcast_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en);
int rdd_mcast_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en);
int rdd_local_switching_ingress_filters_cfg(rdd_port_profile_t port_profile_idx, bdmf_boolean enable);
int rdd_ip_fragments_ingress_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action);
int rdd_hdr_err_ingress_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action);
int rdd_src_mac_anti_spoofing_lookup_cfg(rdd_port_profile_t port_profile_idx, bdmf_boolean enable);
int rdd_src_mac_anti_spoofing_entry_add(rdd_port_profile_t port_profile_idx, uint32_t src_mac_prefix);
int rdd_src_mac_anti_spoofing_entry_delete(rdd_port_profile_t port_profile_idx, uint32_t src_mac_prefix);
int rdd_ingress_filter_ip_address_entry_add(bdmf_ip_t ip_address);
int rdd_ingress_filter_ip_address_entry_delete(bdmf_ip_t ip_address);

/*** Port Profile ***/
void rdd_port_profile_map_set(rdpa_traffic_dir dir, rdd_vport_id_t port_idx, rdd_port_profile_t profile_idx);
rdd_port_profile_t rdd_port_profile_map_get(rdpa_traffic_dir dir, rdd_vport_id_t port_idx);

#endif
