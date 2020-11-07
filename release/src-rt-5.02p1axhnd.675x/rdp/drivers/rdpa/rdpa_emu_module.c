/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

/*******************************************************************
 * rdpa_module_init.c
 *
 * Runner Data Path API - init code
 * This file must be given to the linker first!
 *
 *******************************************************************/

#define DEBUG

#include <bdmf_dev.h>
#include "rdpa_api.h"
#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#include "drv_shell.h"
#endif

static int _aggr_type_reg(struct bdmf_aggr_type *at)
{
    int rc, real_trace_level;

    real_trace_level = bdmf_global_trace_level;
    bdmf_global_trace_level = bdmf_trace_level_debug;
    rc = bdmf_attr_aggregate_type_register(at);
    BDMF_TRACE_INFO("Registered aggregate type (%d): %-16s : %s\n",
        rc, at->name, at->help ? at->help : "");
    bdmf_global_trace_level = real_trace_level;
    return rc;
}

/* extern files from system */
extern struct bdmf_aggr_type system_ruuner_ext_sw_type;
extern struct bdmf_aggr_type system_init_config_type;
extern struct bdmf_aggr_type system_tpid_detect_cfg_type;
extern struct bdmf_aggr_type system_counter_cfg_type;
extern struct bdmf_aggr_type system_config_type;
extern struct bdmf_aggr_type system_sw_version;
extern struct bdmf_aggr_type system_us_stat_type;
extern struct bdmf_aggr_type system_ds_stat_type;
extern struct bdmf_aggr_type system_common_stat_type;
extern struct bdmf_aggr_type system_stat_type;
extern struct bdmf_aggr_type dp_key_type;
extern struct bdmf_aggr_type system_tod_type;
extern struct bdmf_aggr_type system_qm_config_type;
extern struct bdmf_aggr_type system_packet_buffer_config_type;
extern struct bdmf_aggr_type debug_stat_type;
extern struct bdmf_aggr_type system_packet_buffer_config_type;
extern struct bdmf_aggr_type system_ug_alloc_cfg_type;
extern struct bdmf_aggr_type system_ug_rsv_thresh;
extern struct bdmf_aggr_type system_wlan_rsv_thresh;
extern struct bdmf_aggr_type natc_cntr_type;
extern struct bdmf_aggr_type system_resources_type;
extern struct bdmf_type system_drv; 


static int rdpa_system_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&system_ruuner_ext_sw_type);
    rc = rc ? : _aggr_type_reg(&system_init_config_type);
    rc = rc ? : _aggr_type_reg(&system_tpid_detect_cfg_type);
    rc = rc ? : _aggr_type_reg(&system_counter_cfg_type);
    rc = rc ? : _aggr_type_reg(&system_config_type);
    rc = rc ? : _aggr_type_reg(&system_sw_version);
    rc = rc ? : _aggr_type_reg(&system_us_stat_type);
    rc = rc ? : _aggr_type_reg(&system_ds_stat_type);
    rc = rc ? : _aggr_type_reg(&system_common_stat_type);
    rc = rc ? : _aggr_type_reg(&system_stat_type);
    rc = rc ? : _aggr_type_reg(&dp_key_type);
    rc = rc ? : _aggr_type_reg(&system_tod_type);
    rc = rc ? : _aggr_type_reg(&system_qm_config_type);
    rc = rc ? : _aggr_type_reg(&debug_stat_type);
    rc = rc ? : _aggr_type_reg(&system_ug_alloc_cfg_type);
    rc = rc ? : _aggr_type_reg(&system_ug_rsv_thresh);
    rc = rc ? : _aggr_type_reg(&system_wlan_rsv_thresh);
    rc = rc ? : _aggr_type_reg(&system_packet_buffer_config_type);
    rc = rc ? : _aggr_type_reg(&system_resources_type);
    rc = rc ? : _aggr_type_reg(&natc_cntr_type);

    rc = rc ? : bdmf_type_register(&system_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, system_drv.name,
        system_drv.description);

    return rc;
}

#if defined(BCM63158)
extern struct bdmf_aggr_type l2_flow_key_type;
extern struct bdmf_aggr_type l2_flow_result_type;
extern struct bdmf_aggr_type l2_flow_info_type;
extern struct bdmf_type l2_ucast_drv;

static int rdpa_l2_ucast_class_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&l2_flow_key_type);
    rc = rc ? : _aggr_type_reg(&l2_flow_result_type);
    rc = rc ? : _aggr_type_reg(&l2_flow_info_type);
    rc = rc ? : bdmf_type_register(&l2_ucast_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, l2_ucast_drv.name,
        l2_ucast_drv.description);

    return rc;
}


extern struct bdmf_aggr_type ip_flow_key_type;
extern struct bdmf_aggr_type ip_flow_result_type;
extern struct bdmf_aggr_type ip_flow_info_type;
extern struct bdmf_aggr_type ipv4_host_address_table_type;
extern struct bdmf_aggr_type ipv6_host_address_table_type;
extern struct bdmf_aggr_type ip_addresses_table_type;
extern struct bdmf_aggr_type host_mac_address_table_type;
extern struct bdmf_aggr_type fc_accel_mode_type;
extern struct bdmf_aggr_type ds_wan_udp_filter_type;
extern struct bdmf_type ucast_drv;
extern struct bdmf_aggr_type mapt_cfg_type;


static int rdpa_ucast_class_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&ip_flow_key_type);
    rc = rc ? : _aggr_type_reg(&ip_flow_result_type);
    rc = rc ? : _aggr_type_reg(&ip_flow_info_type);
    rc = rc ? : _aggr_type_reg(&ipv4_host_address_table_type);
    rc = rc ? : _aggr_type_reg(&ipv6_host_address_table_type);
    rc = rc ? : _aggr_type_reg(&ip_addresses_table_type);
    rc = rc ? : _aggr_type_reg(&host_mac_address_table_type);
    rc = rc ? : _aggr_type_reg(&fc_accel_mode_type);
    rc = rc ? : _aggr_type_reg(&ds_wan_udp_filter_type);
    rc = rc ? : _aggr_type_reg(&mapt_cfg_type);
    rc = rc ? : bdmf_type_register(&ucast_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, ucast_drv.name,
        ucast_drv.description);

    return rc;
}


extern struct bdmf_aggr_type blog_logical_port_type;
extern struct bdmf_aggr_type rdpa_blog_info_type;
extern struct bdmf_aggr_type blog_ip_tuple_shared_type;
extern struct bdmf_aggr_type blog_ipv4_tuple_type;
extern struct bdmf_aggr_type blog_ipv6_tuple_type;
extern struct bdmf_aggr_type blog_header_type;
extern struct bdmf_aggr_type blog_entry_type;
extern struct bdmf_aggr_type rdpa_blog_rule_filter_type;
extern struct bdmf_aggr_type rdpa_blog_rule_type;
extern struct bdmf_type blog_drv;

static int rdpa_blog_module_init(void)
{
    int rc = BDMF_ERR_OK;

    rc = rc ? : _aggr_type_reg(&blog_logical_port_type);
    rc = rc ? : _aggr_type_reg(&rdpa_blog_info_type);
    rc = rc ? : _aggr_type_reg(&blog_ip_tuple_shared_type);
    rc = rc ? : _aggr_type_reg(&blog_ipv4_tuple_type);
    rc = rc ? : _aggr_type_reg(&blog_ipv6_tuple_type);
    rc = rc ? : _aggr_type_reg(&blog_header_type);
    rc = rc ? : _aggr_type_reg(&blog_entry_type);
    rc = rc ? : _aggr_type_reg(&rdpa_blog_rule_filter_type);
    rc = rc ? : _aggr_type_reg(&rdpa_blog_rule_type);
    rc = rc ? : bdmf_type_register(&blog_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, blog_drv.name,
        blog_drv.description);

    return rc;
}

#else
extern struct bdmf_aggr_type ip_flow_key_type;
extern struct bdmf_aggr_type ip_flow_result_type;
extern struct bdmf_aggr_type ip_flow_info_type;
extern struct bdmf_aggr_type l4_filter_cfg_type;
extern struct bdmf_type ip_class_drv;


extern struct bdmf_aggr_type mapt_cfg_type;

static int rdpa_ip_class_module_init(void)
{
    int rc;
    rc = _aggr_type_reg(&mapt_cfg_type);
    rc = _aggr_type_reg(&ip_flow_key_type);
    rc = rc ? : _aggr_type_reg(&ip_flow_result_type);
    rc = rc ? : _aggr_type_reg(&ip_flow_info_type);
    rc = rc ? : _aggr_type_reg(&l4_filter_cfg_type);

    rc = rc ? : bdmf_type_register(&ip_class_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, ip_class_drv.name,
        ip_class_drv.description);

    return rc;
}

extern struct bdmf_type bridge_drv;

extern struct bdmf_aggr_type bridge_cfg_type;
extern struct bdmf_aggr_type fdb_key_type;
extern struct bdmf_aggr_type fdb_data_type;
extern struct bdmf_aggr_type bridge_fdb_limit_type;

static int rdpa_bridge_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&bridge_cfg_type);
    rc = rc ? : _aggr_type_reg(&fdb_key_type);
    rc = rc ? : _aggr_type_reg(&fdb_data_type);
    rc = rc ? : _aggr_type_reg(&bridge_fdb_limit_type);

    rc = rc ? : bdmf_type_register(&bridge_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, bridge_drv.name,
        bridge_drv.description);

    return rc;
}

#endif

extern struct bdmf_aggr_type port_dp_type;
extern struct bdmf_aggr_type port_sa_limit_type;
extern struct bdmf_aggr_type port_tm_type;
extern struct bdmf_aggr_type port_stat_type;
extern struct bdmf_aggr_type port_flow_control_type;
extern struct bdmf_aggr_type port_mirror_cfg_type;
extern struct bdmf_aggr_type port_vlan_isolation_cfg_type;
extern struct bdmf_aggr_type port_loopback_conf;
extern struct bdmf_aggr_type port_debug_stat_type;
extern struct bdmf_aggr_type filter_ctrl_type;
extern struct bdmf_aggr_type port_ingress_rate_limit_type;
extern struct bdmf_aggr_type port_pkt_size_stat_type;
extern struct bdmf_type port_drv;



static int rdpa_port_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&port_dp_type);
    rc = rc ? : _aggr_type_reg(&port_sa_limit_type);
    rc = rc ? : _aggr_type_reg(&port_tm_type);
    rc = rc ? : _aggr_type_reg(&port_stat_type);
    rc = rc ? : _aggr_type_reg(&port_flow_control_type);
    rc = rc ? : _aggr_type_reg(&port_mirror_cfg_type);
    rc = rc ? : _aggr_type_reg(&port_vlan_isolation_cfg_type);
    rc = rc ? : _aggr_type_reg(&port_loopback_conf);
    rc = rc ? : _aggr_type_reg(&filter_ctrl_type);
    rc = rc ? : _aggr_type_reg(&port_debug_stat_type);
    rc = rc ? : _aggr_type_reg(&port_pkt_size_stat_type);
    rc = rc ? : _aggr_type_reg(&port_ingress_rate_limit_type);

    rc = rc ? : bdmf_type_register(&port_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, port_drv.name,
        port_drv.description);

    return rc;
}

extern struct bdmf_aggr_type tm_rl_cfg_type;
extern struct bdmf_aggr_type tm_prio_class_cfg_type;
extern struct bdmf_aggr_type tm_queue_cfg_type;
extern struct bdmf_aggr_type tm_queue_index_type;
extern struct bdmf_aggr_type service_queue_cfg_type;
extern struct bdmf_aggr_type rdpa_tm_queue_location_type;
extern struct bdmf_type egress_tm_drv;

static int rdpa_egress_tm_module_init(void)
{
    int rc;
    rc = _aggr_type_reg(&tm_rl_cfg_type);
    rc = rc ? : _aggr_type_reg(&tm_prio_class_cfg_type);
    rc = rc ? : _aggr_type_reg(&tm_queue_cfg_type);
    rc = rc ? : _aggr_type_reg(&tm_queue_index_type);
    rc = rc ? : _aggr_type_reg(&service_queue_cfg_type);
    rc = rc ? : _aggr_type_reg(&rdpa_tm_queue_location_type);

    rc = rc ? : bdmf_type_register(&egress_tm_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, egress_tm_drv.name,
        egress_tm_drv.description);

    return rc;
}

extern struct bdmf_aggr_type cpu_rxq_cfg_type;
extern struct bdmf_aggr_type cpu_rx_stat_type;
extern struct bdmf_aggr_type cpu_meter_cfg_type;
extern struct bdmf_aggr_type cpu_reason_cfg_type;
extern struct bdmf_aggr_type cpu_reason_index_type;
extern struct bdmf_aggr_type cpu_tx_info_type;
extern struct bdmf_aggr_type cpu_tx_dump_type;
extern struct bdmf_aggr_type cpu_tx_stat_type;
extern struct bdmf_type cpu_drv;

static int rdpa_cpu_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&cpu_rxq_cfg_type);
    rc = rc ? : _aggr_type_reg(&cpu_rx_stat_type);
    rc = rc ? : _aggr_type_reg(&cpu_meter_cfg_type);
    rc = rc ? : _aggr_type_reg(&cpu_reason_cfg_type);
    rc = rc ? : _aggr_type_reg(&cpu_reason_index_type);
    rc = rc ? : _aggr_type_reg(&cpu_tx_info_type);
    rc = rc ? : _aggr_type_reg(&cpu_tx_dump_type);
    rc = rc ? : _aggr_type_reg(&cpu_tx_stat_type);

    rc = rc ? : bdmf_type_register(&cpu_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, port_drv.name,
        port_drv.description);

    return rc;
}

extern struct bdmf_aggr_type rdpa_stat_aggr_type;
extern struct bdmf_aggr_type rdpa_stat_1_way_aggr_type;
extern struct bdmf_aggr_type rdpa_stat_tx_rx_aggr_type;
extern struct bdmf_aggr_type rdpa_dir_index_aggr_type;

static int rdpa_types_init(void)
{
    int rc;
    rc = _aggr_type_reg(&rdpa_stat_aggr_type);
    rc = rc ? : _aggr_type_reg(&rdpa_stat_1_way_aggr_type);
    rc = rc ? : _aggr_type_reg(&rdpa_stat_tx_rx_aggr_type);
    rc = rc ? : _aggr_type_reg(&rdpa_dir_index_aggr_type);

    return rc;
}

extern struct bdmf_aggr_type classification_result_type;

static int rdpa_common_init(void)
{
    int rc;
    rc = _aggr_type_reg(&classification_result_type);

    return rc;
}


#if !defined(BCM63158)
extern struct bdmf_aggr_type vlan_action_type;
extern struct bdmf_type vlan_action_drv;

static int rdpa_vlan_action_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&vlan_action_type);

    rc = rc ? : bdmf_type_register(&vlan_action_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, vlan_action_drv.name,
        vlan_action_drv.description);

    return rc;
}

extern struct bdmf_type tcont_drv;

static int rdpa_tcont_module_init(void)
{
    int rc;

    rc = bdmf_type_register(&tcont_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, tcont_drv.name,
        tcont_drv.description);

    return rc;
}

extern struct bdmf_type pbit_to_gem_drv;

static int rdpa_pbit_to_gem_module_init(void)
{
    int rc;

    rc = bdmf_type_register(&pbit_to_gem_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, pbit_to_gem_drv.name,
        pbit_to_gem_drv.description);

    return rc;
}

extern struct bdmf_aggr_type gem_stat_type;
extern struct bdmf_aggr_type gem_us_cfg_type;
extern struct bdmf_aggr_type ds_cfg_type;
extern struct bdmf_aggr_type gem_port_action_type;
extern struct bdmf_type gem_drv;

static int rdpa_gem_init(void)
{
    int rc;
    rc = _aggr_type_reg(&gem_stat_type);
    rc = rc ? : _aggr_type_reg(&gem_us_cfg_type);
    rc = rc ? : _aggr_type_reg(&ds_cfg_type);
    rc = rc ? : _aggr_type_reg(&gem_port_action_type);

    rc = rc ? : bdmf_type_register(&gem_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, gem_drv.name,
        gem_drv.description);

    return rc;
}

extern struct bdmf_aggr_type llid_port_action_type;
extern struct bdmf_aggr_type llid_stat_type;
extern struct bdmf_type llid_drv;

static int rdpa_llid_init(void)
{
    int rc;
    rc = _aggr_type_reg(&llid_port_action_type);
    rc = rc ? : _aggr_type_reg(&llid_stat_type);

    rc = rc ? : bdmf_type_register(&llid_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, llid_drv.name,
        llid_drv.description);

    return rc;
}

extern struct bdmf_aggr_type mllid_stat_type;
extern struct bdmf_type mllid_drv;

static int rdpa_mllid_init(void)
{
    int rc;
    rc = rc ? : _aggr_type_reg(&mllid_stat_type);

    rc = rc ? : bdmf_type_register(&mllid_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, mllid_drv.name,
        mllid_drv.description);

    return rc;
}

#ifndef BCM6856
extern struct bdmf_aggr_type generic_rule_type;
extern struct bdmf_aggr_type class_configuration_type;
extern struct bdmf_aggr_type classification_key_type;
extern struct bdmf_aggr_type classification_info_type;
extern struct bdmf_aggr_type port_action_key_type;
extern struct bdmf_aggr_type port_action_type;
extern struct bdmf_type ingress_class_drv;

static int rdpa_ingress_class_module_init(void)
{
    int rc;

    rc = _aggr_type_reg(&generic_rule_type);
    rc = rc ? : _aggr_type_reg(&class_configuration_type);
    rc = rc ? : _aggr_type_reg(&classification_key_type);
    rc = rc ? : _aggr_type_reg(&classification_info_type);
    rc = rc ? : _aggr_type_reg(&port_action_key_type);
    rc = rc ? : _aggr_type_reg(&port_action_type);

    rc = rc ? : bdmf_type_register(&ingress_class_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, ingress_class_drv.name,
        ingress_class_drv.description);

    return rc;
}

extern struct bdmf_aggr_type iptv_channel_key_type;
extern struct bdmf_aggr_type iptv_channel_request_type;
extern struct bmdf_aggr_type iptv_channel_type;
extern struct bdmf_aggr_type iptv_stat_type;
extern struct bdmf_aggr_type channel_req_key_type;
extern struct bdmf_type iptv_drv; 

static int rdpa_iptv_init(void)
{
    int rc;
    rc = _aggr_type_reg(&iptv_channel_key_type);
    rc = rc ? : _aggr_type_reg(&iptv_channel_request_type);
    rc = rc ? : _aggr_type_reg(&channel_req_key_type);
    rc = rc ? : _aggr_type_reg(&iptv_channel_type);
    rc = rc ? : _aggr_type_reg(&iptv_stat_type);

    rc = rc ? : bdmf_type_register(&iptv_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, iptv_drv.name,
        iptv_drv.description);


    return rc;
}
#endif /* #ifndef BCM6856 */
#endif /* #if !defined(BCM63158) */

#if !defined(BCM63158)
extern struct bdmf_aggr_type tm_policer_cfg_type;
extern struct bdmf_aggr_type tm_policer_stat_type;
extern struct bdmf_type policer_drv;

static int rdpa_policer_init(void)
{
    int rc;
    rc = _aggr_type_reg(&tm_policer_cfg_type);
    rc = rc ? : _aggr_type_reg(&tm_policer_stat_type);

    rc = rc ? : bdmf_type_register(&policer_drv);
    BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n", rc, policer_drv.name,
        policer_drv.description);


    return rc;
}
#endif

int rdpa_module_init(void)
{
    int rc;
    rc = rdpa_system_module_init();
    rc = rc ? rc : rdpa_types_init();
#if !defined(BCM63158)
    rc = rc ? rc : rdpa_policer_init();
    rc = rc ? rc : rdpa_ip_class_module_init();
    rc = rc ? rc : rdpa_bridge_module_init();
#else
    rc = rc ? rc : rdpa_ucast_class_module_init();
    rc = rc ? rc : rdpa_l2_ucast_class_module_init();
#endif
    rc = rc ? rc : rdpa_egress_tm_module_init();
#if !defined(BCM63158)
    rc = rc ? rc : rdpa_vlan_action_module_init();
#endif
    rc = rc ? rc : rdpa_common_init();
#if !defined(BCM63158)
    rc = rc ? rc : rdpa_pbit_to_gem_module_init();
#endif
#if !defined(BCM63158)
    rc = rc ? rc : rdpa_ingress_class_module_init();
#endif
    rc = rc ? rc : rdpa_cpu_module_init();
    rc = rc ? rc : rdpa_port_module_init();
#if !defined(BCM63158)
    rc = rc ? rc : rdpa_tcont_module_init();
    rc = rc ? rc : rdpa_gem_init();
    rc = rc ? rc : rdpa_llid_init();
    rc = rc ? rc : rdpa_mllid_init();
#ifndef BCM6856
    rc = rc ? rc : rdpa_iptv_init();
#endif
#else /* defined(BCM63158) */
    rc = rc ? rc : rdpa_blog_module_init();
#endif
    return rc;
}

void rdpa_module_exit(void)
{
}

module_init(rdpa_module_init);
module_exit(rdpa_module_exit);
MODULE_LICENSE("Proprietary");

MODULE_DESCRIPTION("Runner Data Path API. (C) Broadcom");
