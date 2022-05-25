/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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
int rdd_tpid_detect_filter_cfg(rdd_port_profile_t port_profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable);
void rdd_tpid_detect_filter_value_cfg(rdd_module_t *module, uint16_t tpid_detect_filter_value);
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
