/*
 * <:copyright-BRCM:2013:proprietary:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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
#include "rdpa_int.h"
#include "rdd_ih_defs.h"
#include "rdd.h"
#include "rdd_data_structures.h"
#if !defined(LEGACY_RDP) && !defined(WL4908)
#include "rdd_ip_flow.h"
#include "rdd_l4_filters.h"
#else
#include "rdpa_rdd_inline.h"
#endif
#if !defined(WL4908)
#include "rdd_tunnels_parsing.h"
#endif
#include <rdp_drv_ih.h>
#include "rdpa_egress_tm_inline.h"
#include "rdpa_int.h"
#include "rdpa_ip_class_int.h"
#include "rdpa_port_int.h"
#include "rdpa_flow_idx_pool.h"
#include <rdpa_tunnel.h>

int ip_class_prepare_rdd_ip_flow_params(rdpa_ip_flow_info_t *const info,
    rdd_fc_context_t *rdd_ip_flow_ctx, bdmf_boolean is_new_flow);

#define RDPA_FC_VARIABLE_ACTIONS (rdpa_fc_action_forward | rdpa_fc_action_dscp_remark | \
    rdpa_fc_action_gre_remark | rdpa_fc_action_opbit_remark | rdpa_fc_action_ipbit_remark | rdpa_fc_action_policer)

#define RDPA_IH_DA_FILTER_WAN_MAC 3

static uint32_t ds_lite_flows_num;

extern struct bdmf_object *ip_class_object;

#ifndef LEGACY_RDP
extern rdd_module_t us_ip_flow;
extern rdd_module_t ds_ip_flow;
extern rdd_module_t ds_tunnels_parsing;
#endif

extern struct rdp_v6_subnets rdp_v6_subnets[rdpa_ds_lite_max_tunnel_id+1];

static uint32_t ds_lite_flows_num;
static uint32_t gre_flows_num;

const bdmf_attr_enum_table_t rdpa_l4_filter_index_enum_table =
{
    .type_name = "rdpa_l4_filter_index", .help = "L4 filter index",
    .values = {
        {"icmp", rdpa_l4_filter_icmp}, /**<ICMP protocol filter */
        {"esp", rdpa_l4_filter_esp}, /**<ESP protocol filter */
        {"gre", rdpa_l4_filter_gre}, /**<GRE protocol filter */
        {"ah", rdpa_l4_filter_ah}, /**<AH protocol filter */
        {"user_0", rdpa_l4_filter_user_def_0}, /**< User defined l4 protocol  filter  */
        {"user_1", rdpa_l4_filter_user_def_1}, /**< User defined l4 protocol  filter  */
        {"user_2", rdpa_l4_filter_user_def_2}, /**< User defined l4 protocol  filter  */
        {"user_3", rdpa_l4_filter_user_def_3}, /**< User defined l4 protocol  filter  */
        /**<Default filter action  when traffic is non tcpudp and none of the other filters matches  */
        {"non_tcp_udp", rdpa_l4_filter_non_tcp_udp_default_filter},
        {NULL, 0}
    }
};

static inline bdmf_boolean rdpa_is_user_def_l4_filter(rdpa_l4_filter_index filter)
{
    return (filter >= rdpa_l4_filter_user_def_0) && (filter <= rdpa_l4_filter_user_def_3);
}

static inline bdmf_boolean rdpa_is_tunnel_filter(rdpa_l4_filter_index filter)
{
    return filter == rdpa_l4_filter_gre || filter == rdpa_l4_filter_esp;
}

/***************************************************************************
 * RDD interface
 ***************************************************************************/

/* Configure l4 filter in RDD and IH */
static int ip_class_l4_filter_rdd_cfg(bdmf_object_handle mo, bdmf_index index, rdpa_l4_filter_cfg_t *l4_filter_cfg)
{
#define PROTOCOL_DONT_CARE 0xFFFF

    rdd_l4_filter_t rdd_filter = RDD_L4_FILTER_UNKNOWN;
    rdd_action rdd_filter_action;
    uint8_t rdd_filter_parameter;
    uint32_t l4_protocol;

    /* Validate: Action 'Forward' - GRE/ESP filter only */
    if (l4_filter_cfg->action == rdpa_forward_action_forward)
    {
        if (!rdpa_is_tunnel_filter(index)) /* Not GRE nor ESP */
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                "Filter %d cannot be configured with 'Forward' action\n", (int) index);
        }
    }

    switch (index)
    {
    case rdpa_l4_filter_user_def_0:
    case rdpa_l4_filter_user_def_1:
    case rdpa_l4_filter_user_def_2:
    case rdpa_l4_filter_user_def_3:
        rdd_filter = RDD_L4_FILTER_UDEF_0 + (index - rdpa_l4_filter_user_def_0);
        rdd_filter_parameter = rdpa_cpu_rx_reason_l4_udef_0 + (index - rdpa_l4_filter_user_def_0);
        l4_protocol = l4_filter_cfg->protocol;
        break;
    case rdpa_l4_filter_icmp:
        rdd_filter = RDD_L4_FILTER_ICMP;
        rdd_filter_parameter = rdpa_cpu_rx_reason_l4_icmp;
        l4_protocol = IPPROTO_ICMP;
        break;
    case rdpa_l4_filter_esp:
        rdd_filter = RDD_L4_FILTER_ESP;
        rdd_filter_parameter = rdpa_cpu_rx_reason_l4_esp;
        l4_protocol = IPPROTO_ESP;
        break;
    case rdpa_l4_filter_gre:
        rdd_filter = RDD_L4_FILTER_GRE;
        rdd_filter_parameter = rdpa_cpu_rx_reason_l4_gre;
        l4_protocol = IPPROTO_GRE;
        break;
    case rdpa_l4_filter_ah:
        rdd_filter = RDD_L4_FILTER_AH;
        rdd_filter_parameter = rdpa_cpu_rx_reason_l4_ah;
        l4_protocol = IPPROTO_AH;
        break;
    case rdpa_l4_filter_non_tcp_udp_default_filter:
        rdd_filter = RDD_L4_FILTER_UNKNOWN;
        rdd_filter_parameter = rdpa_cpu_rx_reason_non_tcp_udp;
        l4_protocol = PROTOCOL_DONT_CARE;
        break;
    default:
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Unsupported L4 filter: %d\n", (int) index);
    }

    /* Map action */
    rdd_filter_action =
        (l4_filter_cfg->action == rdpa_forward_action_forward) ? ACTION_FORWARD :
        (l4_filter_cfg->action == rdpa_forward_action_host) ? ACTION_TRAP :
        ACTION_DROP;


    /* Call RDD API */

    /* Downstream */
    rdd_l4_filter_set(rdd_filter, rdd_filter_action, rdd_filter_parameter, rdpa_dir_ds);

    /* Upstream */

    /* Set action - 'Trap'                      */
    /* Except GRE/ESP filter                        */
    /* Do not change the default FW behavior    */
    if (!rdpa_is_tunnel_filter(index)) /* Not GRE/ESP */
        rdd_filter_action = ACTION_TRAP;

    rdd_l4_filter_set(rdd_filter, rdd_filter_action, rdd_filter_parameter, rdpa_dir_us);

    /* User-defined protocols must be configured in AH */
    if (rdpa_is_user_def_l4_filter(index))
    {
        int ih_error;
        ih_error = fi_bl_drv_ih_set_user_ip_l4_protocol(index - rdpa_l4_filter_user_def_0, l4_protocol);
        if (ih_error)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "ih error %d\n", (int)ih_error);
    }
    return 0;
}

int ip_class_attr_l4_filter_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_l4_filter_cfg_t *l4_filter_cfg = (rdpa_l4_filter_cfg_t *)val;
    int rc;

    /* Validate protocol for udef filters */
    if (rdpa_is_user_def_l4_filter(index))
    {
        if (l4_filter_cfg->protocol == IPPROTO_ICMP ||
            l4_filter_cfg->protocol == IPPROTO_GRE  ||
            l4_filter_cfg->protocol == IPPROTO_ESP  ||
            l4_filter_cfg->protocol == IPPROTO_AH   ||
            l4_filter_cfg->protocol == IPPROTO_IPV6 ||
            l4_filter_cfg->protocol == IPPROTO_ICMP ||
            l4_filter_cfg->protocol == IPPROTO_TCP  ||
            l4_filter_cfg->protocol == IPPROTO_UDP  ||
            l4_filter_cfg->protocol == IPPROTO_IGMP ||
            l4_filter_cfg->protocol == IPPROTO_ICMPV6)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "User-defined protocol %d is invalid\n",
                (int)l4_filter_cfg->protocol);
        }
    }

    /* Notify RDD, return error if error */
    rc = ip_class_l4_filter_rdd_cfg(mo, index, l4_filter_cfg);
    if (rc)
        return rc;

    /* update driver private data */
    if (rdpa_is_user_def_l4_filter(index))
    {
        /* protocol field is relevant only for user defined protocol L4  filters */
        ip_class->l4_filters[index].protocol = l4_filter_cfg->protocol;
    }
    ip_class->l4_filters[index].action = l4_filter_cfg->action;

    return 0;
}

/* "l4_filter" attribute "read" callback */
int ip_class_attr_l4_filter_cfg_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_l4_filter_cfg_t *l4_filter_cfg = (rdpa_l4_filter_cfg_t *)val;

    if (rdpa_is_user_def_l4_filter(index) &&
        ip_class->l4_filters[index].protocol == RDPA_INVALID_PROTOCOL)
    {
        return BDMF_ERR_NOENT;
    }
    *l4_filter_cfg = ip_class->l4_filters[index];
    return 0;
}

/* Read L4 filter statistics and accumulate in the shadow */
static int ip_class_l4_filter_stat_rdd_get(struct bdmf_object *mo)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    rdd_various_counters_t rdd_counters = {};
    int rdd_rc;

    rdd_rc = rdd_various_counters_get(rdpa_dir_ds, L4_FILTERS_DROP_COUNTER_MASK, 1, &rdd_counters);
    if (rdd_rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "rdd_various_counters_get(rdpa_dir_ds, L4_FILTERS_DROP_COUNTER_MASK, ) -> %d\n", rdd_rc);
    }
    ip_class->l4_filter_stat[rdpa_l4_filter_icmp] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_ICMP];
    ip_class->l4_filter_stat[rdpa_l4_filter_esp] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_ESP];
    ip_class->l4_filter_stat[rdpa_l4_filter_gre] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_GRE];
    ip_class->l4_filter_stat[rdpa_l4_filter_ah] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_AH];
    ip_class->l4_filter_stat[rdpa_l4_filter_user_def_0] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_UDEF_0];
    ip_class->l4_filter_stat[rdpa_l4_filter_user_def_1] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_UDEF_1];
    ip_class->l4_filter_stat[rdpa_l4_filter_user_def_2] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_UDEF_2];
    ip_class->l4_filter_stat[rdpa_l4_filter_user_def_3] += rdd_counters.layer4_filters_drop[RDD_L4_FILTER_UDEF_3];
    ip_class->l4_filter_stat[rdpa_l4_filter_non_tcp_udp_default_filter] +=
        rdd_counters.layer4_filters_drop[RDD_L4_FILTER_UNKNOWN];
    return 0;
}

/* "us_l4_filter_stat" attribute "read" callback */
int  ip_class_attr_l4_filter_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t *stat = (uint32_t *)val;
    int rc;

    /* Collect from RDD */
    rc = ip_class_l4_filter_stat_rdd_get(mo);
    if (rc)
        return rc;

    /* Return and clear */
    *stat = ip_class->l4_filter_stat[index];
    ip_class->l4_filter_stat[index] = 0;

    return 0;
}

/* reads ip flow data from RDD */
static int _ip_class_read_rdd_ip_flow(uint32_t index, rdpa_ip_flow_info_t *info)
{
    rdd_fc_context_t rdd_ip_flow_ctx = {};
    int rc;

    memset(info, 0, sizeof(*info));
    /* TODO , ask what is the ip_class operational method, and read fields matching the operational method */
    /* read the ip flow data from the RDD */
    rc = rdd_context_entry_get(index, &rdd_ip_flow_ctx);
    if (rc)
        return BDMF_ERR_NOENT;

    info->result.qos_method = rdd_ip_flow_ctx.qos_method;
    info->result.action = rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_forward ?
        (rdd_ip_flow_ctx.fwd_action == RDD_FC_FWD_ACTION_CPU ?
            rdpa_forward_action_host : rdpa_forward_action_drop) : rdpa_forward_action_forward;
    info->result.trap_reason = rdd_ip_flow_ctx.trap_reason;
    info->result.dscp_value = rdd_ip_flow_ctx.ip_version ? /* is_ipv6: compute TOS field */
        ((rdd_ip_flow_ctx.dscp_value << ECN_IN_TOS_SHIFT) | rdd_ip_flow_ctx.ecn_value) :
        rdd_ip_flow_ctx.dscp_value;
    info->result.nat_port = rdd_ip_flow_ctx.nat_port;
    if (!rdd_ip_flow_ctx.ip_version) /* if ipv4 */
        info->result.nat_ip.addr.ipv4 = rdd_ip_flow_ctx.nat_ip.addr.ipv4;
#ifndef LEGACY_RDP
    info->result.port = rdd_vport_to_rdpa_if(rdd_ip_flow_ctx.vir_egress_port, rdd_ip_flow_ctx.wifi_ssid);
#else
    info->result.drop_eligibility = rdd_ip_flow_ctx.drop_eligibility;
    info->result.port = rdpa_rdd_bridge_port_to_if(rdd_ip_flow_ctx.egress_port, rdd_ip_flow_ctx.wifi_ssid);
#endif
    info->result.queue_id = BDMF_INDEX_UNASSIGNED;
    info->result.wan_flow = rdd_ip_flow_ctx.wan_flow_index;
    if (rdd_ip_flow_ctx.conn_dir == rdpa_dir_ds)
    {
        if (rdpa_if_is_cpu_port(info->result.port))
        {
            info->result.wl_metadata = rdd_ip_flow_ctx.wl_metadata;
        }
        else
        {
            _rdpa_egress_tm_queue_id_by_lan_port_queue(info->result.port, rdd_ip_flow_ctx.traffic_class,
                &info->result.queue_id);
        }
    }
    else
    {
        if (rdpa_if_is_wifi(info->result.port))
        {
            info->result.wl_metadata = rdd_ip_flow_ctx.wl_metadata;
            if (rdd_ip_flow_ctx.wfd.nic_ucast.is_wfd)
            {
                /* XXX: Add support for 2 queues (retrieve queue_id from both wfd idx and wfd prio */
                info->result.queue_id = rdd_ip_flow_ctx.wfd.nic_ucast.wfd_idx;
            }
            else
            {
                info->result.queue_id = rdd_ip_flow_ctx.rnr.priority;
            }
        }
        else if (rdpa_if_is_cpu_port(info->result.port))
        {
            info->result.wl_metadata = rdd_ip_flow_ctx.wl_metadata;
        }
        else
        {
            int wan_flow = info->result.wan_flow;

            _rdpa_egress_tm_queue_id_by_wan_flow_rc_queue(&wan_flow, rdd_ip_flow_ctx.rate_controller,
                rdd_ip_flow_ctx.traffic_class, &info->result.queue_id);
            info->result.wan_flow = wan_flow;
        }
    }

    if (rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_policer)
    {
        int policer_idx = rdd_ip_flow_ctx.policer_id;
        rdpa_policer_key_t key = { .dir = rdd_ip_flow_ctx.conn_dir, .index = policer_idx };
        bdmf_object_handle policer;
        int rc;

        rc = rdpa_policer_get(&key, &policer);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find policer %d\n", policer_idx);
        bdmf_put(policer);

        info->result.policer_obj = policer;
    }

    if ((rdd_ip_flow_ctx.conn_dir == rdpa_dir_us) && (rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_dslite_tunnel))
    {
        info->result.ds_lite_src = rdp_v6_subnets[rdd_ip_flow_ctx.ds_lite_hdr_index].src;
        info->result.ds_lite_dst = rdp_v6_subnets[rdd_ip_flow_ctx.ds_lite_hdr_index].dst;
    }

    if ((rdd_ip_flow_ctx.conn_dir == rdpa_dir_us) && (rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_gre_tunnel))
    {
        bdmf_object_handle tunnel;
        int rc;

        rc = rdpa_tunnel_get(rdd_ip_flow_ctx.tunnel_index, &tunnel);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find tunnel %d\n", rdd_ip_flow_ctx.tunnel_index);
        bdmf_put(tunnel);
        info->result.tunnel_obj = tunnel;
    }

    info->result.ovid_offset = rdd_ip_flow_ctx.ovid_offset;
    info->result.opbit_action = rdd_ip_flow_ctx.opbit_action;
    info->result.ipbit_action = rdd_ip_flow_ctx.ipbit_action;
    info->result.l2_header_offset = rdd_ip_flow_ctx.l2_hdr_offset - DRV_RDD_IH_PACKET_HEADER_OFFSET;
    info->result.l2_header_size = rdd_ip_flow_ctx.l2_hdr_size;
    info->result.l2_header_number_of_tags = rdd_ip_flow_ctx.l2_hdr_number_of_tags;
    info->result.action_vec = rdd_ip_flow_ctx.actions_vector;
    info->result.action_vec &= ~rdpa_fc_action_policer; /* policer action is used only by RDD */
    info->result.service_q_id = BDMF_INDEX_UNASSIGNED;
    if (rdd_ip_flow_ctx.service_queue_enabled)
    {
        info->result.action_vec |= rdpa_fc_action_service_q;
        info->result.service_q_id = egress_tm_svcq_queue_id_get(rdd_ip_flow_ctx.service_queue_id);
    }
    memcpy(info->result.l2_header, rdd_ip_flow_ctx.l2_header, rdd_ip_flow_ctx.l2_hdr_size);
    rdd_connection_entry_get(rdd_ip_flow_ctx.conn_dir, rdd_ip_flow_ctx.conn_index, &info->key, &index);
    info->key.dir = rdd_ip_flow_ctx.conn_dir;
    return 0;
}

/* "flow" attribute delete callback */
int ip_class_attr_flow_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    int rc;
    rdd_fc_context_t rdd_ip_flow_ctx = {};
#ifndef LEGACY_RDP
    rdd_module_t *ip_flow_module;
#endif
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    rc = rdd_context_entry_get(rdd_flow_id, &rdd_ip_flow_ctx);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "IP flow context retrieval failed, error=%d\n", rc);
#ifndef LEGACY_RDP
    ip_flow_module = rdd_ip_flow_ctx.conn_dir == rdpa_dir_us ? &us_ip_flow : &ds_ip_flow;
#endif

    /* read the ip flow data from the RDD */
    if ((rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_dslite_tunnel) && (ds_lite_flows_num != 0))
    {
        if (rdd_ip_flow_ctx.conn_dir == rdpa_dir_us)
            --rdp_v6_subnets[rdd_ip_flow_ctx.ds_lite_hdr_index].refcnt;
        if (!--ds_lite_flows_num)
        {
            rdpa_l4_filter_cfg_t l4_filter_cfg = { rdpa_forward_action_host, RDPA_INVALID_PROTOCOL };

            ip_class_attr_l4_filter_cfg_write(mo, NULL, rdpa_l4_filter_user_def_3,
                &l4_filter_cfg, sizeof(l4_filter_cfg));

            rdd_tunnels_parsing_enable(&tunnels_parsing, 1);
        }
    }

#ifndef LEGACY_RDP
    rc = rdd_ip_flow_delete(ip_flow_module, rdd_flow_id);
#else
    rc = rdd_connection_entry_delete(rdd_flow_id);
#endif
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "IP flow deletion failed, error=%d\n", rc);

    if ((rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_gre_tunnel) && (gre_flows_num != 0))
    {
        bdmf_object_handle tunnel;
        int rc;

        rc = rdpa_tunnel_get(rdd_ip_flow_ctx.tunnel_index, &tunnel);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot find tunnel %d\n", rdd_ip_flow_ctx.tunnel_index);

        if (rdd_ip_flow_ctx.conn_dir == rdpa_dir_us && tunnel)
            _rdpa_tunnel_ref_count_decrease(tunnel);
        bdmf_put(tunnel);
        gre_flows_num--;
    }

    if (((rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_dslite_tunnel) && ds_lite_flows_num == 0) ||
        ((rdd_ip_flow_ctx.actions_vector & rdpa_fc_action_gre_tunnel) && gre_flows_num == 0))
#if !defined(LEGACY_RDP)
        rdd_tunnels_parsing_enable(ip_flow_module, 0);
#else
        rdd_oren_tunnels_parsing_enable(0);
#endif
    return 0;
}

/*
 * ip_class attribute access
 */

/* "flow" attribute "read" callback */
int ip_class_attr_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    int rc;

    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the ip flow data from the RDD */
    rc = _ip_class_read_rdd_ip_flow(rdd_flow_id, info);
    if (rc)
        BDMF_TRACE_ERR("Failed to _ip_class_read_rdd_ip_flow for rdd_flow_id %u\n", rdd_flow_id);

    /* store rdd_flow_id */
    info->hw_flow_id = rdd_flow_id;

    return rc;
}

/* "flow" attribute write callback */
int ip_class_attr_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    rdd_fc_context_t rdd_ip_flow_ctx;
    int rc;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the ip flow data from the RDD */
    rc = rdd_context_entry_get(rdd_flow_id, &rdd_ip_flow_ctx);
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to rdd_context_entry_get for rdd_flow_id %u\n", rdd_flow_id);
        return BDMF_ERR_NOENT;
    }

    /* prepare ip flow data to modify in RDD */
    rc = ip_class_prepare_rdd_ip_flow_params(info, &rdd_ip_flow_ctx, 0);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Modifying ip flow parameters failed, error %d\n", rc);

    /* modify the ip flow in the RDD */
#ifndef LEGACY_RDP
    rc = rdd_context_entry_modify(rdd_flow_id, &rdd_ip_flow_ctx);
#else
    rc = rdd_context_entry_modify(&rdd_ip_flow_ctx, rdd_flow_id);
#endif
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Modifying ip flow failed, error %d\n", rc);

    /* ToDo: for legacy mode if subnet changed, verify that exists and reference/unreference */

    return 0;
}

static int ip_flow_key_validate(rdpa_ip_flow_key_t *key)
{
    if (key->prot != IPPROTO_TCP && key->prot != IPPROTO_UDP)
    {
        if (key->prot == IPPROTO_GRE)
        {
            if (key->src_port)
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Source port specified for GRE protocol (%d)\n", key->prot);
        }
        else if (key->prot != IPPROTO_ESP && (key->src_port || key->dst_port))
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "%s port specified for non TCP/UDP protocol (%d)\n",
                key->src_port ? "Source" : "Destination", key->prot);
        }
    }
    else
    {
        if (!key->src_port || !key->dst_port)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Missing %s port\n", key->dst_port ? "Source" : "Destination");
    }

    return 0;
}

/* "flow" attribute add callback */
int ip_class_attr_flow_add_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size, bdmf_index *ctx_ext_index)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(ip_class_object);
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    int rc;
    bdmf_boolean tunnel_enable;
    rdd_ip_flow_t rdd_ip_flow = {};
#ifndef LEGACY_RDP
    rdd_module_t *ip_flow_module = info->key.dir == rdpa_dir_us ? &us_ip_flow : &ds_ip_flow;
#endif

    rc = ip_flow_key_validate(&info->key);
    if (rc)
        return rc;

    /* prepare ip flow result to configure in RDD */
    rc = ip_class_prepare_rdd_ip_flow_params(info, &rdd_ip_flow.context_entry, 1);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Modifying ip flow parameters failed, error %d\n", rc);

    /* create the ip flow in the RDD */
    rdd_ip_flow.lookup_entry = &info->key;

    tunnel_enable = (rdd_ip_flow.context_entry.actions_vector & (rdpa_fc_action_dslite_tunnel | rdpa_fc_action_gre_tunnel)) ? 1 : 0;

    if (tunnel_enable && (ds_lite_flows_num == 0) && (gre_flows_num == 0))
#if !defined(LEGACY_RDP)
        rdd_tunnels_parsing_enable(&ds_tunnels_parsing, 1);
#else
        rdd_oren_tunnels_parsing_enable(1);
#endif
    if ((rdd_ip_flow.context_entry.actions_vector & rdpa_fc_action_dslite_tunnel) && (ds_lite_flows_num == 0))
    {
        rdpa_l4_filter_cfg_t l4_filter_cfg = { rdpa_forward_action_host, IPPROTO_IPIP };

        if (_rdpa_system_cfg_get()->headroom_size < RDPA_DS_LITE_HEADROOM_SIZE)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Headroom size less then the required for DS-lite header (40)\n");

        /* configure L4 user defined filter #3 to ipv4 (as required by FW) */
        if (ip_class->l4_filters[rdpa_l4_filter_user_def_3].protocol != RDPA_INVALID_PROTOCOL)
            BDMF_TRACE_RET(BDMF_ERR_NORES, "DS-Lite: Layer 4 protocol user_defined_3 already in use\n");

        rc = ip_class_attr_l4_filter_cfg_write(mo, NULL, rdpa_l4_filter_user_def_3, &l4_filter_cfg,
            sizeof(l4_filter_cfg));
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set layer 4 protocol user_defined_3 for DS-lite\n");
    }
#ifndef LEGACY_RDP
    rc = rdd_ip_flow_add(ip_flow_module, &info->key, &rdd_ip_flow.context_entry, &rdd_ip_flow.xo_entry_index);
#else
    rc = rdd_connection_entry_add(&rdd_ip_flow, info->key.dir);
#endif
    if (rc)
    {
        if ((rdd_ip_flow.context_entry.actions_vector & rdpa_fc_action_dslite_tunnel) && (ds_lite_flows_num == 0))
        {
            rdpa_l4_filter_cfg_t l4_filter_cfg = { rdpa_forward_action_host, RDPA_INVALID_PROTOCOL };

            ip_class_attr_l4_filter_cfg_write(mo, NULL, rdpa_l4_filter_user_def_3, &l4_filter_cfg,
                sizeof(l4_filter_cfg));
        }
        if (tunnel_enable && (ds_lite_flows_num == 0) && (gre_flows_num == 0))
#if !defined(LEGACY_RDP)
            rdd_tunnels_parsing_enable(&ds_tunnels_parsing, 0);
#else
            rdd_oren_tunnels_parsing_enable(0);
#endif
        if (rc == BDMF_ERR_ALREADY || rc == BDMF_ERR_NORES)
            return rc;
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "IP flow creation failed, error=%d\n", rc);
    }

    if (rdd_ip_flow.context_entry.actions_vector & rdpa_fc_action_dslite_tunnel)
    {
        if (info->key.dir)
            ++rdp_v6_subnets[rdd_ip_flow.context_entry.ds_lite_hdr_index].refcnt;
        ++ds_lite_flows_num;
    }

    if (rdd_ip_flow.context_entry.actions_vector & rdpa_fc_action_gre_tunnel)
    {
        ++gre_flows_num;
        if (info->key.dir == rdpa_dir_us && info->result.tunnel_obj)
            _rdpa_tunnel_ref_count_increase(info->result.tunnel_obj);
    }

    /* set the created flow index, to return*/
    *index = rdd_ip_flow.xo_entry_index;

    return 0;
}

/* "flow" attribute find callback.
 * Updates *index, can update *val as well
 */
int ip_class_attr_flow_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size)
{
    rdpa_ip_flow_info_t *info = (rdpa_ip_flow_info_t *)val;
    int rc;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
#ifndef LEGACY_RDP
    rdd_module_t *ip_flow_module = info->key.dir == rdpa_dir_us ? &us_ip_flow : &ds_ip_flow;

    rc = rdd_ip_flow_find(ip_flow_module, &info->key, &rdd_flow_id);
    if (rc)
        return rc;
    rdpa_flow_id = rdpa_build_flow_id(rdd_flow_id, RDPA_FLOW_TUPLE_L3);

    if (rdpa_flow_idx_pool_reverse_get_index(ip_class->flow_idx_pool_p, &rdpa_flow_idx, rdpa_flow_id))
    {
        BDMF_TRACE_ERR("Failed to find rdpa_flow_idx for rdd_flow_id %u\n", rdd_flow_id);
        return BDMF_ERR_NOENT; /* Must never happen */
    }
    /* return rdpa_flow_idx to caller */
    *index = rdpa_flow_idx;
    return 0;
#else
    BL_LILAC_RDD_ADD_CONNECTION_DTE  rdd_ip_flow = {};

    rdd_ip_flow.lookup_entry = &info->key;

    /* search the ip flow  in RDD, TODO remove casting when RDD is ready */
    rc = rdd_connection_entry_search(&rdd_ip_flow, info->key.dir, &rdd_flow_id);
    if (rc)
        return BDMF_ERR_NOENT;
    rdpa_flow_id = rdpa_build_flow_id(rdd_flow_id, RDPA_FLOW_TUPLE_L3);

    if (rdpa_flow_idx_pool_reverse_get_index(ip_class->flow_idx_pool_p, &rdpa_flow_idx, rdpa_flow_id))
    {
        BDMF_TRACE_ERR("Failed to find rdpa_flow_idx for rdd_flow_id %u\n", rdd_flow_id);
        return BDMF_ERR_NOENT; /* Must never happen */
    }
    /* return rdpa_flow_idx to caller */
    *index = rdpa_flow_idx;
    return 0;
#endif
}

/* "flow_stat" attribute "read" callback */
int ip_class_attr_flow_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    rdd_fc_context_t rdd_ip_flow_ctx = {};
    int rc;
    uint32_t rdpa_flow_idx = index;
    uint32_t rdpa_flow_id;
    uint32_t rdd_flow_id;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);

    if (rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3))
    {
        return BDMF_ERR_NOENT;
    }

    /* read the ip flow result from the RDD */
    rc = rdd_context_entry_get(rdd_flow_id, &rdd_ip_flow_ctx);
    if (rc)
        return BDMF_ERR_NOENT;

    stat->packets = rdd_ip_flow_ctx.valid_cnt.packets;
    stat->bytes = rdd_ip_flow_ctx.valid_cnt.bytes;
    return BDMF_ERR_OK;
}

void ip_class_destroy_ex(struct bdmf_object *mo)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    rdd_ic_context_t context = {};

    if (ip_class_object != mo)
        return;
    if (ip_class->op_method != rdpa_method_none)
    {
        /* if working with flow cache, remove internal ingress context */

        /* configure internal l2 flow to drop */
        context.action = rdpa_forward_action_drop;
        context.policer = BDMF_INDEX_UNASSIGNED;
        context.rate_shaper = BDMF_INDEX_UNASSIGNED;

        rc = rdd_ic_context_cfg(rdpa_dir_ds, RDPA_FC_DS_IC_RESULT_ID, &context);
        if (rc)
            BDMF_TRACE_ERR("Removing internal ingress context from RDD failed, error %d\n", rc);
    }

    /* Reset all filters to trap traffic to the host */
    for (i = 0; i < rdpa_l4_filter__num_of; i++)
    {
        ip_class->l4_filters[i].action = rdpa_forward_action_host;
        ip_class_l4_filter_rdd_cfg(mo, i, &ip_class->l4_filters[i]);
    }
}

/* "fc_bypass" attribute "write" callback */
int ip_class_attr_fc_bypass_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    bdmf_error_t rc;
    int i;
    uint32_t ipv4_mc_subnet, ipv4_mc_subnet_mask;
    uint32_t ipv6_mc_subnet_1, ipv6_mc_subnet_2, ipv6_mc_subnet_mask;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t mask = *(uint32_t *)val;
    bdmf_boolean multi_filter_enable = 0;
    bdmf_object_handle filter_obj;

    if (mask & RDPA_IP_CLASS_MASK_BP_ALL)
       BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Bypass All is not applicible in this platform\n");
    rdd_local_switching_fc_enable(RDD_EMAC_ID_START, ((mask & RDPA_IP_CLASS_MASK_US_WLAN) == 0) ? 1 : 0);

    rc = rdpa_filter_get(&filter_obj);
    if (!rc)
    {
        rdpa_ports wan_ports = RDPA_PORT_ALL_WAN;

        while (1)
        {
            port_drv_priv_t *wan_port;
            rdpa_if port_idx = rdpa_port_get_next(&wan_ports);

            if (port_idx == rdpa_if_none)
                break;
            if (!port_objects[port_idx])
                continue;
            wan_port = (port_drv_priv_t *)bdmf_obj_data(port_objects[port_idx]);
            if (wan_port->ingress_filters[RDPA_FILTER_MCAST].enabled)
            {
                multi_filter_enable = 1;
                /* if multicast filter is enabled, setting rdpa_ip_class_fc_bypass_set
                   to RDPA_IP_CLASS_MC_IP should be rejected and vice versa */
                if (mask & RDPA_IP_CLASS_MASK_MC_IP)
                {
                    bdmf_put(filter_obj);
                    BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
                        "can't configure fc_bypass=multicast_ip when wan filter multicast exists");
                }
                break;
            }
        }
        bdmf_put(filter_obj);
    }

    /* in case of multicast ip */
    /* Ipv4 multicast range - 224.0.0.0 to 239.255.255.255, mask = 240.0.0.0. Runner expects host-order */

    ipv4_mc_subnet = 0xe0000000;
    ipv4_mc_subnet_mask = 0xf0000000;
    /* Ipv6 multicast range - FF00:: to FF0F:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF */
    /*                        and FF30:: to FF3F:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF  */
    /*                        mask = FFF0:: */
    ipv6_mc_subnet_1 = 0xff000000;
    ipv6_mc_subnet_2 = 0xff300000;
    ipv6_mc_subnet_mask = 0xfff00000;

    rdd_full_fc_acceleration_cfg(RDD_FULL_FC_ACCELERATION_NON_IP, rdpa_dir_ds,
        (mask & RDPA_IP_CLASS_MASK_PURE_MAC) >> RDPA_IP_CLASS_PURE_MAC);
    rdd_full_fc_acceleration_cfg(RDD_FULL_FC_ACCELERATION_NON_IP, rdpa_dir_us,
        (mask & RDPA_IP_CLASS_MASK_PURE_MAC) >> RDPA_IP_CLASS_PURE_MAC);
    rdd_full_fc_acceleration_cfg(RDD_FULL_FC_ACCELERATION_MCAST_IP, rdpa_dir_ds,
        (mask & RDPA_IP_CLASS_MASK_MC_IP) >> RDPA_IP_CLASS_MC_IP);
    rdd_full_fc_acceleration_cfg(RDD_FULL_FC_ACCELERATION_MCAST_IP, rdpa_dir_us,
        (mask & RDPA_IP_CLASS_MASK_MC_IP) >> RDPA_IP_CLASS_MC_IP);

    /* when no multi filter on wan0, disable mc_ip from by pass mode will disable all ip filters */
    if (multi_filter_enable == 0)
    {
        fi_bl_drv_ih_set_ip_filter(0, ipv4_mc_subnet, ipv4_mc_subnet_mask, DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);
        fi_bl_drv_ih_set_ip_filter(1, ipv6_mc_subnet_1, ipv6_mc_subnet_mask, DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);
        fi_bl_drv_ih_set_ip_filter(2, ipv6_mc_subnet_2, ipv6_mc_subnet_mask, DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);

        for (i = 0; i < DRV_IH_NUMBER_OF_IP_FILTERS - 1; ++i)
            fi_bl_drv_ih_enable_ip_filter(i, (mask & RDPA_IP_CLASS_MASK_MC_IP) >> RDPA_IP_CLASS_MC_IP);
    }
    /* Configure Pure IP mode */
    rdd_3_tupples_ip_flows_enable(mask & RDPA_IP_CLASS_MASK_PURE_IP);
    ip_class->fc_bypass_mask = mask;

    return 0;
}

/* "key mask" attribute "write" callback */
int ip_class_attr_key_type_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{    
    return BDMF_ERR_NOT_SUPPORTED;
}

void ip_class_pre_init_ex(struct bdmf_object *mo)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    /* Initialize protocols for L4 filters */
    ip_class->l4_filters[rdpa_l4_filter_icmp].protocol = IPPROTO_ICMP;
    ip_class->l4_filters[rdpa_l4_filter_gre].protocol = IPPROTO_GRE;
    ip_class->l4_filters[rdpa_l4_filter_esp].protocol = IPPROTO_ESP;
    ip_class->l4_filters[rdpa_l4_filter_ah].protocol = IPPROTO_AH;
    for (i = rdpa_l4_filter_user_def_0; i <= rdpa_l4_filter_user_def_3; i++)
        ip_class->l4_filters[i].protocol = RDPA_INVALID_PROTOCOL;

    /* Set up all filters to trap traffic to the host by default */
    for (i = 0; i < rdpa_l4_filter__num_of; i++)
    {
        ip_class->l4_filters[i].action = rdpa_forward_action_host;
        if (!rdpa_is_user_def_l4_filter(i))
            ip_class_l4_filter_rdd_cfg(mo, i, &ip_class->l4_filters[i]);
    }
}

void ip_class_post_init_ex(struct bdmf_object *mo)
{
}

/* "routed_mac" attribute "write" callback */
int ip_class_attr_routed_mac_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_mac_t *mac = (bdmf_mac_t *)val;
    bdmf_mac_t zero_mac = {};
    int rc;

    if (!mac)
        mac = &zero_mac;

    rc = fi_bl_drv_ih_set_da_filter_without_mask(RDPA_IH_DA_FILTER_WAN_MAC + index, mac->b);
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Cannot set MAC %pM in IH, ih_error = %d\n", mac, rc);

    rc = fi_bl_drv_ih_enable_da_filter(RDPA_IH_DA_FILTER_WAN_MAC + index, bdmf_mac_is_zero(mac) ? 0 : 1);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Cannot enable filter %d in IH. ih_error = %d\n",
            (int)(RDPA_IH_DA_FILTER_WAN_MAC + index), rc);
    }

    memcpy(&ip_class->routed_mac[index], mac, sizeof(bdmf_mac_t));
    return BDMF_ERR_OK;
}

/* "pathstat" attribute "read" callback */
int ip_class_attr_pathstat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "tcp_ack_prio" attribute "write" callback */
int ip_class_attr_tcp_ack_prio_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

