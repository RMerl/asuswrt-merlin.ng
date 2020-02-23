/*
 * <:copyright-BRCM:2015:proprietary:standard
 *
 *    Copyright (c) 2015 Broadcom
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
 * :>
 */


#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdp_drv_bbh.h"
#include "rdp_drv_ih.h"
#include "rdpa_int.h"
#include "rdd_ih_defs.h"
#include "rdd.h"
#include "rdd_init.h"
#ifndef LEGACY_RDP
#include "rdd_ic.h"
#if !defined(WL4908)
#include "rdd_multicast_processing.h"
#endif
#endif
#include "rdd_tm.h"
#ifdef CONFIG_BCM_GPON_TODD
#include <gpon_tod_gpl.h>
#endif
#include "rdpa_system_ex.h"

#if !defined(BCM_DSL_RDP)
void rdpa_ih_cfg_bcast_classifier(void);
void rdpa_ih_cfg_oam_classifier(void);
#endif

int emac_ports_headroom_hw_cfg(int headroom_size);
int emac_ports_mtu_hw_cfg(int mtu_size);
int _tpid_detect_cfg(struct bdmf_object * const mo, rdpa_tpid_detect_t tpid_detect,
    const rdpa_tpid_detect_cfg_t * const tpid_detect_cfg);

extern int triple_tag_detect_ref_count;
extern int num_wan;
extern int num_lan;
extern struct bdmf_object *system_object;

int headroom_hw_cfg(int headroom_size);
int mtu_hw_cfg(int mtu_size);

int system_post_init_wan(rdpa_wan_type wan_type, rdpa_emac wan_emac)
{
#if defined(__OREN__)
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(system_object);
    int rc;

#if !defined(BDMF_SYSTEM_SIM)
    rc = system_data_path_init_fiber(wan_type);
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, system_object, "Failed system data path init fiber rc=%d\n", rc);
#endif

    rc = headroom_hw_cfg(system->cfg.headroom_size);
    rc = rc ? rc : mtu_hw_cfg(system->cfg.mtu_size);

    return rc;
#else
    return 0;
#endif
}

int system_attr_tod_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    rdpa_system_tod_t *system_tod = (rdpa_system_tod_t *)val;
    int err = BDMF_ERR_NOT_SUPPORTED;

#ifdef CONFIG_BCM_GPON_TODD
    gpon_todd_tstamp_t tstamp;
    uint64_t ts;
#endif /* CONFIG_BCM_GPON_TODD */

    /* Set default: Nullify */
    memset(system_tod, 0, sizeof(rdpa_system_tod_t));

#ifdef CONFIG_BCM_GPON_TODD

    /* Call ToDD API */
    gpon_todd_get_tod(&tstamp, &ts);

    /* Update framework */

    /* Seconds */
    system_tod->sec_ms = tstamp.sec_ms; /* MS */
    system_tod->sec_ls = tstamp.sec_ls; /* LS */

    /* Nanoseconds */
    system_tod->nsec = tstamp.nsec;

    system_tod->ts48_nsec = 0;

    err = 0;
#endif /* CONFIG_BCM_GPON_TODD */

    return err;
}

/* US drop statistics */
struct bdmf_aggr_type system_us_stat_type =
{
    .name = "system_us_stat", .struct_name = "rdpa_system_us_stat_t",
    .help = "System US Drop Statistics", .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        { .name = "eth_flow_action", .help = "Flow action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, eth_flow_action)
        },
        { .name = "sa_lookup_failure", .help = "SA lookup failure", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, sa_lookup_failure)
        },
        { .name = "da_lookup_failure", .help = "DA lookup failure", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, da_lookup_failure)
        },
        { .name = "sa_action", .help = "SA action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, sa_action)
        },
        { .name = "da_action", .help = "DA action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, da_action)
        },
        { .name = "forwarding_matrix_disabled", .help = "Disabled in forwarding matrix", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, forwarding_matrix_disabled)
        },
        { .name = "connection_action", .help = "Connection action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, connection_action)
        },
        { .name = "parsing_exception", .help = "Parsing exception", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, parsing_exception)
        },
        { .name = "parsing_error", .help = "Parsing error", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, parsing_error)
        },
        { .name = "local_switching_congestion", .help = "Local switching congestion", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, local_switching_congestion)
        },
        { .name = "vlan_switching", .help = "VLAN switching", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, vlan_switching)
        },
        { .name = "tpid_detect", .help = "Invalid TPID", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, tpid_detect)
        },
        { .name = "invalid_subnet_ip", .help = "Invalid subnet IP", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, invalid_subnet_ip)
        },
        { .name = "acl_oui", .help = "Dropped by OUI ACL", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, acl_oui)
        },
        { .name = "acl", .help = "Dropped by ACL", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, acl)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(system_us_stat_type);

/* DS drop statistics */
struct bdmf_aggr_type system_ds_stat_type =
{
    .name = "system_ds_stat", .struct_name = "rdpa_system_ds_stat_t",
    .help = "System DS Drop Statistics", .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        { .name = "eth_flow_action", .help = "Flow action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, eth_flow_action)
        },
        { .name = "sa_lookup_failure", .help = "SA lookup failure", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, sa_lookup_failure)
        },
        { .name = "da_lookup_failure", .help = "DA lookup failure", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, da_lookup_failure)
        },
        { .name = "sa_action", .help = "SA action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, sa_action)
        },
        { .name = "da_action", .help = "DA action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, da_action)
        },
        { .name = "forwarding_matrix_disabled", .help = "Disabled in forwarding matrix", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, forwarding_matrix_disabled)
        },
        { .name = "connection_action", .help = "Connection action == drop", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, connection_action)
        },
        { .name = "policer", .help = "Dropped by policer", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, policer)
        },
        { .name = "parsing_exception", .help = "Parsing exception", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, parsing_exception)
        },
        { .name = "parsing_error", .help = "Parsing error", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, parsing_error)
        },
        { .name = "iptv_layer3", .help = "IPTV L3 filter", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, iptv_layer3)
        },
        { .name = "vlan_switching", .help = "VLAN switching", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, vlan_switching)
        },
        { .name = "tpid_detect", .help = "Invalid TPID", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, tpid_detect)
        },
        { .name = "ds_lite_congestion", .help = "DSLite congestion", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, dual_stack_lite_congestion)
        },
        { .name = "invalid_subnet_ip", .help = "Invalid subnet IP", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, invalid_subnet_ip)
        },
        { .name = "invalid_layer2_protocol", .help = "Invalid L2 protocol", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, invalid_layer2_protocol)
        },
        { .name = "firewall", .help = "Firewall", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, firewall)
        },
        { .name = "dst_mac_non_router", .help = "DA doesn't match router MAC", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, dst_mac_non_router)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(system_ds_stat_type);

/* "stat" attribute "read" callback */
int system_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    rdpa_system_stat_t *stat = (rdpa_system_stat_t *)val;
    rdd_various_counters_t rdd_us_stat = {};
    rdd_various_counters_t rdd_ds_stat = {};
    /* all flags, but ingress and L4 filters. Flags that are supported only
     * in 1 direction are ignored by RDD
     */
    static uint32_t stat_req_mask =
        INVALID_L2_PROTO_DROP_COUNTER_MASK |
        FIREWALL_DROP_COUNTER_MASK |
        ACL_OUI_DROP_COUNTER_MASK  |
        DST_MAC_NON_ROUTER_DROP_COUNTER_MASK |
        ETHERNET_FLOW_ACTION_DROP_COUNTER_MASK |
        SA_LOOKUP_FAILURE_DROP_COUNTER_MASK | DA_LOOKUP_FAILURE_DROP_COUNTER_MASK |
        SA_ACTION_DROP_COUNTER_MASK | DA_ACTION_DROP_COUNTER_MASK |
        FORWARDING_MATRIX_DISABLED_DROP_COUNTER_MASK |
        CONNECTION_ACTION_DROP_COUNTER_MASK |
        IPTV_L3_DROP_COUNTER_MASK |
        LOCAL_SWITCHING_CONGESTION_COUNTER_MASK |
        VLAN_SWITCHING_DROP_COUNTER_MASK |
        DOWNSTREAM_POLICERS_DROP_COUNTER_MASK |
        IP_VALIDATION_FILTER_DROP_COUNTER_MASK |
        EMAC_LOOPBACK_DROP_COUNTER_MASK |
        TPID_DETECT_DROP_COUNTER_MASK |
        DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_MASK |
        INVALID_SUBNET_IP_DROP_COUNTER_MASK|
        ACL_L2_DROP_COUNTER_MASK;

    int rdd_rc;

    rdd_rc = rdd_various_counters_get(rdpa_dir_us, stat_req_mask, 1, &rdd_us_stat);
    rdd_rc = rdd_rc ? rdd_rc : rdd_various_counters_get(rdpa_dir_ds, stat_req_mask, 1, &rdd_ds_stat);
    if (rdd_rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "rdd_various_counters_get() --> %d\n", rdd_rc);

    stat->us.eth_flow_action = rdd_us_stat.eth_flow_action_drop;
    stat->us.sa_lookup_failure = rdd_us_stat.sa_lookup_failure_drop;
    stat->us.da_lookup_failure = rdd_us_stat.da_lookup_failure_drop;
    stat->us.sa_action = rdd_us_stat.sa_action_drop;
    stat->us.da_action = rdd_us_stat.da_action_drop;
    stat->us.forwarding_matrix_disabled = rdd_us_stat.forwarding_matrix_disabled_drop;
    stat->us.connection_action = rdd_us_stat.connection_action_drop;
    stat->us.parsing_exception = rdd_ds_stat.ip_validation_filter_drop[1];
    stat->us.parsing_error = rdd_ds_stat.ip_validation_filter_drop[0];
    stat->us.local_switching_congestion = rdd_us_stat.local_switching_congestion;
    stat->us.vlan_switching = rdd_us_stat.vlan_switching_drop;
    stat->us.tpid_detect = rdd_us_stat.tpid_detect_drop;
    stat->us.invalid_subnet_ip = rdd_us_stat.invalid_subnet_ip_drop;
    stat->us.acl_oui = rdd_us_stat.acl_oui_drop;
    stat->us.acl = rdd_us_stat.acl_l2_drop; /*both l2 and l3 dropped packets are counted in this parameter */

    stat->ds.eth_flow_action = rdd_ds_stat.eth_flow_action_drop;
    stat->ds.sa_lookup_failure = rdd_ds_stat.sa_lookup_failure_drop;
    stat->ds.da_lookup_failure = rdd_ds_stat.da_lookup_failure_drop;
    stat->ds.sa_action = rdd_ds_stat.sa_action_drop;
    stat->ds.da_action = rdd_ds_stat.da_action_drop;
    stat->ds.forwarding_matrix_disabled = rdd_ds_stat.forwarding_matrix_disabled_drop;
    stat->ds.connection_action = rdd_ds_stat.connection_action_drop;
    stat->ds.policer = rdd_ds_stat.downstream_policers_drop;
    stat->ds.parsing_exception = rdd_ds_stat.ip_validation_filter_drop[1];
    stat->ds.parsing_error = rdd_ds_stat.ip_validation_filter_drop[0];
    stat->ds.iptv_layer3 = rdd_ds_stat.iptv_layer3_drop;
    stat->ds.vlan_switching = rdd_ds_stat.vlan_switching_drop;
    stat->ds.tpid_detect = rdd_ds_stat.tpid_detect_drop;
    stat->ds.dual_stack_lite_congestion = rdd_ds_stat.dual_stack_lite_congestion_drop;
    stat->ds.invalid_subnet_ip = rdd_ds_stat.invalid_subnet_ip_drop;
    stat->ds.invalid_layer2_protocol = rdd_ds_stat.invalid_layer2_protocol_drop;
    stat->ds.firewall = rdd_ds_stat.firewall_drop;
    stat->ds.dst_mac_non_router = rdd_ds_stat.dst_mac_non_router_drop;

    return 0;
}

int system_attr_debug_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    /*not supported in RDP*/
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "stat" attribute "write" callback */
int system_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    /*not supported in RDP*/
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "debug_stat" attribute "write" callback */
int system_attr_debug_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    /*not supported in RDP*/
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_system_cfg_t *cfg = (rdpa_system_cfg_t *)val;
#ifndef WL4908
    uint16_t inner_tpid = cfg->inner_tpid;
    uint16_t outer_tpid = cfg->outer_tpid;
    uint16_t add_always_tpid = cfg->add_always_tpid;
#endif
#ifndef RUNNER_CPU_DQM_TX
    int i;
#endif

    int rc = 0;

    if (cfg->mtu_size < RDPA_MIN_MTU || cfg->mtu_size > RDPA_MTU)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "MTU out of range\n");

    if (cfg->cpu_redirect_mode != rdpa_rx_redirect_to_cpu_disabled)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "RX Redirect to CPU is not supported\n");

    if (cfg->headroom_size != system->cfg.headroom_size)
    {
        rc = headroom_hw_cfg(cfg->headroom_size);
        if (rc)
            return rc;
#ifndef RUNNER_CPU_DQM_TX
        /* Initialize pbuf support */
        bdmf_pbuf_init(rdpa_bpm_buffer_size_get(), DRV_RDD_IH_PACKET_HEADER_OFFSET + cfg->headroom_size);

        for (i = 0; i < bdmf_sysb_type__num_of; i++)
            bdmf_sysb_headroom_size_set(i, DRV_RDD_IH_PACKET_HEADER_OFFSET + cfg->headroom_size);
#endif
    }

    if (cfg->mtu_size != system->cfg.mtu_size)
    {
        rc = mtu_hw_cfg(cfg->mtu_size);
        if (rc)
            return rc;
    }

#ifndef WL4908
    rdd_ic_debug_mode_enable(cfg->ic_dbg_stats);
    if ((inner_tpid != system->cfg.inner_tpid) || (outer_tpid != system->cfg.outer_tpid))
        rdd_egress_ethertype_config(inner_tpid, outer_tpid);

    if (add_always_tpid != system->cfg.add_always_tpid)
        rdd_vlan_command_always_egress_ether_type_config(add_always_tpid);
#endif

#if defined(__OREN__)
    rdd_force_dscp_to_pbit_config(rdpa_dir_us, cfg->force_dscp_to_pbit_us);
    rdd_force_dscp_to_pbit_config(rdpa_dir_ds, cfg->force_dscp_to_pbit_ds);
#endif

    return 0;
}

#ifdef G9991
#define RPDA_MAX_GEM_FLOW (256 - G9991_SID_PORTS_DS)
static int g9991_create_cpu_ds_flows(void)
{
    int rc, i;

    for (i = 0; i < G9991_SID_PORTS_DS; i++)
    {
        rdpa_ic_result_t cfg =
        {
            .action = rdpa_forward_action_forward,
            .forw_mode = rdpa_forwarding_mode_flow,
            .egress_port = rdpa_if_lan0 + i,
            .queue_id = 0
        };
        rdd_ic_context_t new_context = {};

        rc = rdpa_map_to_rdd_classifier(rdpa_dir_ds, &cfg, &new_context, 0, 0,
            RDPA_IC_TYPE_FLOW, 1);
        rc = rc ? : rdd_ic_context_cfg(rdpa_dir_ds, RDPA_USER_MAX_DS_IC_RESULTS + i, &new_context);
        rc = rc ? : rdd_ds_wan_flow_cfg(RPDA_MAX_GEM_FLOW + i, 0, 0, RDPA_USER_MAX_DS_IC_RESULTS + i);
    }
    bdmf_trace("%s creating %d default DS flows\n", rc ? "Error" : "Done", i);

    return rc;
}
#endif

int system_post_init_enumerate_emacs(struct bdmf_object *mo)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(mo);
#if !defined(BCM_DSL_RDP) && !defined(BCM63158)
    rdpa_system_init_cfg_t *init_cfg = &system->init_cfg;
#endif

#if defined(__OREN__)
    if (init_cfg->gbe_wan_emac == rdpa_emac5)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "GBE WAN EMAC5 must be created only via rdpa_port\n");
#endif

#if defined(DSL_63138) || defined(DSL_63148)
    system->cfg.car_mode = 1;
    num_lan = 7;
    num_wan = 2;
#elif defined(WL4908)
    system->cfg.car_mode = 1;
    num_lan = 5;
    num_wan = 1;
#elif defined(BCM63158)
    system->cfg.car_mode = 1;
    num_lan = 6;
    num_wan = 1; /* FIXME : MULTI_WAN DSL - should be increased to 3 */
#else
    num_wan = 1;
    num_lan = __bitcount(init_cfg->enabled_emac);

    /* Enable GBE WAN for EMAC0-4. EMAC5 will be enabled only by creating port/index=wan0->wantype with gbe */
    if (init_cfg->gbe_wan_emac != rdpa_emac_none && init_cfg->gbe_wan_emac != rdpa_emac5)
    {
        init_cfg->enabled_emac |= (1 << init_cfg->gbe_wan_emac);
    }

    init_cfg->enabled_emac &= ~(rdpa_emac_id(rdpa_emac5));
#endif

    return 0;
}

int system_pre_init_ex(struct bdmf_object *mo)
{
    return 0;
}

int system_post_init_ex(struct bdmf_object *mo)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

    if (system->cfg.mtu_size < RDPA_MIN_MTU || system->cfg.mtu_size > RDPA_MTU)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "MTU out of range\n");

#ifdef G9991
    rc = rc ? : g9991_create_cpu_ds_flows();
    if (rc)
        return BDMF_ERR_INTERNAL;
#endif

#if defined(WL4908)
    /* set drop precedence table */
    rdd_drop_precedence_cfg(rdpa_dir_ds, system->dp_bitmask[rdpa_dir_ds]);
    rdd_drop_precedence_cfg(rdpa_dir_us, system->dp_bitmask[rdpa_dir_us]);
    rc = headroom_hw_cfg(system->cfg.headroom_size);
#else 
    rdd_ic_debug_mode_enable(system->cfg.ic_dbg_stats);
    if (system->init_cfg.switching_mode != rdpa_switching_none)
    {
        int rdd_rc = 0;
        int rdd_bridge_port;

        for (rdd_bridge_port = BL_LILAC_RDD_LAN0_BRIDGE_PORT;
            rdd_bridge_port <= BL_LILAC_RDD_LAN4_BRIDGE_PORT; rdd_bridge_port++)
        {
            rdd_vlan_switching_isolation_config(rdd_bridge_port, rdpa_dir_ds, 1);
            rdd_vlan_switching_isolation_config(rdd_bridge_port, rdpa_dir_us, 1);
        }

        /* Enable vlan aware/switching in RDD */
        if (system->init_cfg.switching_mode != rdpa_switching_none)
        {
            rdd_vlan_switching_config(1, system->init_cfg.switching_mode == rdpa_mac_based_switching);

            /* Reserve VID index 0 for untagged MAC entries */
            rdd_rc = _rdpa_handle_rdd_lan_vid_entry(0, 1, NULL, NULL);
        }
        if (rdd_rc)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Failed to configure vlan switching mode in RDD. rdd_rc=%d\n",
                rdd_rc);
        }
    }

    /* set drop precedence table */
    rdd_drop_precedence_cfg(rdpa_dir_ds, system->dp_bitmask[rdpa_dir_ds]);
    rdd_drop_precedence_cfg(rdpa_dir_us, system->dp_bitmask[rdpa_dir_us]);
    rc = headroom_hw_cfg(system->cfg.headroom_size);
#endif
    rc = rc ? rc : mtu_hw_cfg(system->cfg.mtu_size);
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "%d: Failed system data path init rc=%d\n", __LINE__, rc);

#if defined(__OREN__)
    rdd_force_dscp_to_pbit_config(rdpa_dir_ds, system->cfg.force_dscp_to_pbit_ds);
    rdd_force_dscp_to_pbit_config(rdpa_dir_us, system->cfg.force_dscp_to_pbit_us);
    rdpa_ih_cfg_bcast_classifier();
    rdpa_ih_cfg_oam_classifier();
#endif

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    /* by default, disable it */
    rdd_ih_congestion_threshold_write(0);
#endif

    return 0;
}

/* TPID Detect */
int _tpid_detect_cfg(struct bdmf_object * const mo,
    rdpa_tpid_detect_t tpid_detect,
    const rdpa_tpid_detect_cfg_t * const tpid_detect_cfg)
{
    system_drv_priv_t *priv = (system_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t ethertype_0;
    uint16_t ethertype_1;
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING ethertype_index;
    DRV_IH_ERROR err_ih;

    /* Handle user-defined */
    if ((tpid_detect == rdpa_tpid_detect_udef_1) ||
        (tpid_detect == rdpa_tpid_detect_udef_2))
    {
        err_ih = fi_bl_drv_ih_get_ethertypes_for_qtag_identification(&ethertype_0, &ethertype_1);
        if (err_ih != DRV_IH_NO_ERROR)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to get user-defined TPIDs Detect values\n");
        }

        if (tpid_detect == rdpa_tpid_detect_udef_1)
        {
            ethertype_0 = tpid_detect_cfg->val_udef;
        }
        else
        {
            ethertype_1 = tpid_detect_cfg->val_udef;
        }

        err_ih = fi_bl_drv_ih_set_ethertypes_for_qtag_identification(ethertype_0, ethertype_1);
        if (err_ih != DRV_IH_NO_ERROR)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to set user-defined TPID Detect values\n");
        }

        /* Update data */
        priv->tpids_detect[tpid_detect].val_udef = tpid_detect_cfg->val_udef;
    }

    /* Map: RDPA -> IH */
    switch (tpid_detect)
    {
    case rdpa_tpid_detect_0x8100:
        {
            ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_8100;
            break;
        }
    case rdpa_tpid_detect_0x88A8:
        {
            ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_88A8;
            break;
        }
    case rdpa_tpid_detect_0x9100:
        {
            ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9100;
            break;
        }
    case rdpa_tpid_detect_0x9200:
        {
            ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9200;
            break;
        }
    case rdpa_tpid_detect_udef_1:
        {
            ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_0;
            break;
        }
    case rdpa_tpid_detect_udef_2:
        {
            ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_1;
            break;
        }
    default:
        {
            ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_8100;
            break;
        }
    }

    /* Call IH API */
    err_ih = fi_bl_drv_ih_configure_qtag_nesting(ethertype_index,
        tpid_detect_cfg->otag_en, tpid_detect_cfg->itag_en);
    if (err_ih != DRV_IH_NO_ERROR)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to configure TPID Detect: %u\n", tpid_detect);
    }

    /* Enable/Disable triple tag detection */
    if (!is_sa_mac_use())
    {
        err_ih = fi_bl_drv_ih_configure_parser_core_cfg_eng_3rd_tag_detection(tpid_detect_cfg->triple_en,
            (ethertype_index));
        if (err_ih != DRV_IH_NO_ERROR)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to configure most inner TPID Detection\n");
        }

        /* Handle the SA operations ref counter */
        if (priv->tpids_detect[tpid_detect].triple_en)
        {
            if (!tpid_detect_cfg->triple_en)
                triple_tag_detect_ref_count--;
        }
        else
        {
            if (tpid_detect_cfg->triple_en)
                triple_tag_detect_ref_count++;
        }
    }
    else if (tpid_detect_cfg->triple_en)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Can not configure most inner TPID Detect with SA MAC lookup\n");

    /* Update data */
    priv->tpids_detect[tpid_detect].otag_en = tpid_detect_cfg->otag_en;
    priv->tpids_detect[tpid_detect].itag_en = tpid_detect_cfg->itag_en;
    priv->tpids_detect[tpid_detect].triple_en = tpid_detect_cfg->triple_en;
    return 0;
}

#if !defined(BCM_DSL_RDP)
/* Configure a classifier for broadcast traffic arriving on IPTV flow */
void rdpa_ih_cfg_bcast_classifier(void)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;

    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask = RDPA_IPTV_FILTER_MASK_BCAST;
    ih_classifier_config.resulting_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    ih_classifier_config.broadcast_da_indication = 1;

    fi_bl_drv_ih_configure_classifier(RDPA_IH_CLASSIFIER_BCAST_IPTV, &ih_classifier_config);
}

void rdpa_ih_cfg_oam_classifier(void)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;

    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask = RDPA_MASK_IH_CLASS_L2_UDEF_2_3;
    ih_classifier_config.l2_protocol = DRV_IH_L2_PROTOCOL_USER_DEFINED_2;
    ih_classifier_config.resulting_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_HIGH_INDEX;

    fi_bl_drv_ih_configure_classifier(RDPA_IH_CLASSIFIER_L2_PROTOCOL_UDEF_2_3, &ih_classifier_config);
}
#endif

/* Update EMAC and BBH Frame length */
int emac_ports_headroom_hw_cfg(int headroom_size)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    const rdpa_system_init_cfg_t *init_cfg = _rdpa_system_init_cfg_get();
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration = {};
    DRV_BBH_PORT_INDEX bbh_port_index;
    rdpa_emac emac;

    if (headroom_size % 8)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Headroom size must be a multiple of 8\n");

    for (emac = rdpa_emac0; emac < rdpa_emac__num_of; emac++)
    {
        if (!(init_cfg->enabled_emac & (1 << emac)))
            continue;

        bbh_port_index = rdpa_emac2bbh_emac(emac);
        fi_bl_drv_bbh_rx_get_configuration(bbh_port_index, &bbh_rx_configuration);
        bbh_rx_configuration.reassembly_offset_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(headroom_size);

        rc = fi_bl_drv_bbh_rx_set_configuration(bbh_port_index, &bbh_rx_configuration);
        if (rc)
            break;
    }
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set headroom size to RDD, error %d\n", rc);
    return rc;
}

/* Update EMAC and BBH Frame length */
int emac_ports_mtu_hw_cfg(int mtu_size)
{
    bdmf_error_t rc = 0;
    const rdpa_system_init_cfg_t *init_cfg = _rdpa_system_init_cfg_get();
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration = {};
    DRV_BBH_PORT_INDEX bbh_port_index;
    rdpa_emac emac;

    for (emac = rdpa_emac0; emac < rdpa_emac__num_of; emac++)
    {
        if (!(init_cfg->enabled_emac & (1 << emac)))
            continue;

        /* Update BBH frame length */
        bbh_port_index = rdpa_emac2bbh_emac(emac);

        fi_bl_drv_bbh_rx_get_configuration(bbh_port_index, &bbh_rx_configuration);
#if RDPA_BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX == 0
        bbh_rx_configuration.maximum_packet_size_0 = mtu_size;
#else
#error problem with BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX
#endif
        /* maximum_packet_size 1-3 are not in use */
        bbh_rx_configuration.maximum_packet_size_1 = mtu_size;
        bbh_rx_configuration.maximum_packet_size_2 = mtu_size;
        bbh_rx_configuration.maximum_packet_size_3 = mtu_size;

        rc = fi_bl_drv_bbh_rx_set_configuration(bbh_port_index, &bbh_rx_configuration);
        if (rc)
            break;
    }
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set MTU size to RDD, error %d\n", rc);
    return 0;
}

bdmf_boolean rdpa_is_ddr_offload_enable(rdpa_traffic_dir dir)
{
    const rdpa_system_init_cfg_t *init_cfg = _rdpa_system_init_cfg_get();

    if (dir == rdpa_dir_ds)
    {
        if (rdpa_is_fttdp_mode())
            return 1;
        else
            return 0;
    }

    /* rdpa_dir_us */
    return init_cfg->us_ddr_queue_enable;
}

int system_attr_cpu_reason_to_tc_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_cpu_reason_to_tc_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "counter_cfg" attribute "write" callback */
int system_counter_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_fpm_isr_delay_timer_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_fpm_isr_delay_timer_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_natc_counter_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_natc_counter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_ih_cong_threshold_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    return rdd_ih_congestion_threshold_read((uint8_t *)val);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int system_attr_ih_cong_threshold_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    return rdd_ih_congestion_threshold_write(*(uint8_t *)val);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int system_attr_ing_cong_ctrl_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_ing_cong_ctrl_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_clock_gate_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
     return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_clock_gate_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
     return BDMF_ERR_NOT_SUPPORTED;
}

int _packet_buffer_cfg(const rdpa_packet_buffer_cfg_t *pb_cfg)
{
    return BDMF_ERR_NOT_SUPPORTED;
}
